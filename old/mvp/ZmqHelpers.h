/// \file ZmqHelpers.h
///
/// Zmq Helper Classes
///
/// TODO: Write something here
///

#ifndef __MVP_ZMQHELPERS_H__
#define __MVP_ZMQHELPERS_H__

#include <vw/Core/Exception.h>

#include <mvp/Config.h>
#include <mvp/MVPJobRequest.pb.h>
#include <mvp/MVPMessages.pb.h>

#include <zmq.hpp>
#include <set>

/*
// Get out of settings later
#define MVP_COMMAND_PORT "6677"
#define MVP_STATUS_PORT "6678"
#define MVP_BROADCAST_PORT "6679"
*/

namespace mvp {

const std::string cmd_sock_url = "tcp://*:" MVP_COMMAND_PORT;
const std::string bcast_sock_url = "tcp://*:" MVP_BROADCAST_PORT;
const std::string status_sock_url = "tcp://*:" MVP_STATUS_PORT;

void sock_send(zmq::socket_t& sock, std::string const& str_message) {
  zmq::message_t reply(str_message.size());
  memcpy(reply.data(), str_message.c_str(), str_message.size());
  sock.send(reply);
}

std::string sock_recv(zmq::socket_t& sock) {
  zmq::message_t message;
  sock.recv(&message);
  return std::string(static_cast<char*>(message.data()), message.size());
}

class ZmqServerHelper {

  zmq::socket_t m_cmd_sock;
  zmq::socket_t m_bcast_sock;
  zmq::socket_t m_status_sock;

  public:
    enum PollEvent {
      STATUS_EVENT,
      COMMAND_EVENT,
    };

    typedef std::set<PollEvent> PollEventSet;

    ZmqServerHelper(zmq::context_t& context) : 
      m_cmd_sock(context, ZMQ_REP),
      m_bcast_sock(context, ZMQ_PUB),
      m_status_sock(context, ZMQ_SUB) {

      m_cmd_sock.bind(cmd_sock_url.c_str());
      m_bcast_sock.bind(bcast_sock_url.c_str());
      m_status_sock.bind(status_sock_url.c_str()); 
    }
  
    PollEventSet poll() {
      PollEventSet events;

      //  Initialize poll set
      zmq::pollitem_t items [] = {
      { m_cmd_sock, 0, ZMQ_POLLIN, 0 },
      { m_status_sock, 0, ZMQ_POLLIN, 0 }
      };

      zmq::poll(items, 2, -1);

      if (items[0].revents & ZMQ_POLLIN) {
        events.insert(COMMAND_EVENT);
      }
      if (items[1].revents & ZMQ_POLLIN) {
        events.insert(STATUS_EVENT);
      }

      return events;
    }

    MVPStatusUpdate recv_status() {
      MVPStatusUpdate update;
      update.ParseFromString(sock_recv(m_status_sock));
      return update;
    }

    MVPCommand recv_cmd() {
      MVPCommand cmd;
      cmd.ParseFromString(sock_recv(m_cmd_sock));
      return cmd;
    }

    void send_cmd(MVPCommandReply const& cmd) {
      std::string str_cmd_message;
      cmd.SerializeToString(&str_cmd_message);
      sock_send(m_cmd_sock, str_cmd_message);
    }

    void send_bcast(MVPWorkerBroadcast::BroadcastType cmd_enum) {
      MVPWorkerBroadcast bcast_cmd;
      bcast_cmd.set_cmd(cmd_enum);

      std::string str_bcast_message;
      bcast_cmd.SerializeToString(&str_bcast_message);
      sock_send(m_bcast_sock, str_bcast_message);
    }
};

class ZmqWorkerHelper {

  zmq::socket_t m_cmd_sock;
  zmq::socket_t m_bcast_sock;
  zmq::socket_t m_status_sock;

  bool m_startup;

  public:
    ZmqWorkerHelper(zmq::context_t& context, std::string const& hostname) :
      m_cmd_sock(context, ZMQ_REQ),
      m_bcast_sock(context, ZMQ_SUB),
      m_status_sock(context, ZMQ_PUB),
      m_startup(true) {

      std::string cmd_sock_url("tcp://" + hostname + ":" MVP_COMMAND_PORT);
      std::string bcast_sock_url("tcp://" + hostname + ":" MVP_BROADCAST_PORT);
      std::string status_sock_url("tcp://" + hostname + ":" MVP_STATUS_PORT);

      m_cmd_sock.connect(cmd_sock_url.c_str());
      m_bcast_sock.connect(bcast_sock_url.c_str());
      m_status_sock.connect(status_sock_url.c_str());

      // Set subscription filter
      m_bcast_sock.setsockopt(ZMQ_SUBSCRIBE, 0, 0); // Don't filter out any messages
    }

    MVPWorkerBroadcast recv_bcast() {
      MVPWorkerBroadcast cmd;

      if (m_startup) {
        cmd.set_cmd(MVPWorkerBroadcast::WAKE);
        m_startup = false;
      } else {
        cmd.ParseFromString(sock_recv(m_bcast_sock));
      }
      return cmd;
    }

    MVPCommandReply get_next_job() {
      MVPCommand cmd;
      cmd.set_cmd(MVPCommand::JOB);
      std::string str_cmd;
      cmd.SerializeToString(&str_cmd);

      sock_send(m_cmd_sock, str_cmd);

      zmq::pollitem_t cmd_poller[] = {{m_cmd_sock, 0, ZMQ_POLLIN, 0}};
      zmq::poll(cmd_poller, 1, 5000);

      if (!(cmd_poller[0].revents & ZMQ_POLLIN)) {
        vw_throw(vw::IOErr() << "Lost connection to mvpd");
      }

      MVPCommandReply reply;
      reply.ParseFromString(sock_recv(m_cmd_sock));

      return reply;
    }
};

} // namespace mvp

#endif

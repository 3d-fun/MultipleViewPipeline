function self = OrbitalImageCollection(_im)
  self = mvpclass();

  self._im = _im;

  self.back_project = @back_project;
endfunction

function result = back_project(self, xyz, orientation, scale, sz)
  morientation = quat2rot(orientation);
  cursor = 1;
  result = {};
  for j = 1:numel(self._im)
    homog = self._im(j).camera * [morientation xyz(:); 0 0 0 1] * [1 0 0; 0 1 0; 0 0 0; 0 0 1] * diag([scale(:);1]) * [eye(2) -sz(:)/2; 0 0 1];
    patch = zeros([flipdim(sz)(:);2]);
    patch(:, :, 1) = _do_homog(self._im(j).data(:, :, 1), homog, sz);
    patch(:, :, 2) = _do_homog(self._im(j).data(:, :, 2), homog, sz);
    if any(patch(:, :, 2))
      result{cursor} = patch;
      cursor += 1;
    endif
  endfor
endfunction

%!function c = camera(center, orientation, focus, sz)
%!  intrinsics = diag([focus(1) -focus(2) 1]);
%!  intrinsics(1:2, 3) = sz / 2;
%!  extrinsics = [orientation -orientation*center(:)];
%!  c = intrinsics * extrinsics;
%!endfunction

%!function b = patcheq(patch1, patch2, tol)
%!  diff = abs(patch1 - patch2);
%!  havevalid = any(patch1);
%!  
%!  sumvalid = sum(diff(:));
%!
%!  b = havevalid && sumvalid < tol;
%!endfunction

%!test
%!  sz = [64, 32];
%!  imdata = rand([fliplr(sz), 2]);
%!
%!  % Camera at z = 1, looking back at origin. 
%!  im(1).data = imdata;
%!  im(1).camera = camera([0 0  1], diag([1 -1 -1]), [1 1], sz); 
%!
%!  % Camera at z = 1, looking back at origin.
%!  im(2).data = imdata;
%!  im(2).camera = camera([0 0 1], diag([1 -1 -1]), [1 1], sz);
%!
%!  % Camera at z = -1, looking toward origin.
%!  im(3).data = imdata;
%!  im(3).camera = camera([0 0 -1], eye(3), [1 1], sz);
%!
%!  oic = OrbitalImageCollection(im);
%!
%!  % Plane at z = 0
%!  patches = oic.back_project([0 0 0], [1 0 0 0], [1 1], sz);
%!
%!  assert(patcheq(patches{1}, imdata, 1e-4));
%!  assert(patcheq(patches{2}, imdata, 1e-4));
%!  assert(!patcheq(patches{3}, imdata, 1e-4));
%!
%!  % Plane at z = -1, patch scale 2m/px
%!  patches = oic.back_project([0 0 -1], [1 0 0 0], [2 2], sz);
%!
%!  assert(patcheq(patches{1}, imdata, 1e-4));
%!  assert(patcheq(patches{2}, imdata, 1e-4));
%!  assert(numel(patches), 2);
%!
%!  % Plane at z = 0, rotated 180deg about x axis (vertical flip) 
%!  patches = oic.back_project([0 0 0], [0 1 0 0], [1 1], sz);
%!
%!  assert(!patcheq(patches{1}, imdata, 1e-4));
%!  assert(!patcheq(patches{2}, imdata, 1e-4));
%!  assert(patcheq(patches{3}, imdata, 1e-4));

% vim:set syntax=octave:

function err = mvperr(patches, hKern, errfun)
  n = numel(patches);
  dim = size(patches{1});

  % TODO: dispatch _mvperr_impl_gauss

  % Normalize patches
  for k = 1:n
    patches{k} = (patches{k} - mean(patches{k}(:))) / std(patches{k}(:));
  endfor

  % Find the albedo
  meanpatch = zeros(dim);
  for k = 1:n
    meanpatch += patches{k};
  endfor
  meanpatch /= n;

  % Find the sum of square error, in a gaussian window
  % TODO: (This should persist somehow)
  gaussrow = normpdf(1:dim(1), dim(1) / 2, dim(1) / 6);
  gausscol = normpdf(1:dim(2), dim(2) / 2, dim(2) / 6)';
  gausswin = gausscol*gaussrow;

  err = 0;
  for k = 1:n
    err += sum((((meanpatch - patches{k}).^2)*gausswin)(:));
  endfor

endfunction

% vim:set syntax=octave:

% function [h, hl] = figureList(fn, h_track)
%     in_h_track = fn == string(fields(h_track));
%     if isempty(h_track)
%         h = 1;
%         hl = nan;
%     elseif any(in_h_track)
%         ht = h_track.(fn);
%         h = ht(1);
%         hl = ht(2);
%     else
        
%     end
% end

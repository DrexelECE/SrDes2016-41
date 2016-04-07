function hulls = FrameSegmenter_RGBLed(im)
im = mat2gray(imread(im));
BW = logical(0*im);
% red = im(:,:,1);
% green = im(:,:,2);
% blue = im(:,:,3);
s = size(im);
for i = 1:s(3)
    seg = channelSegmenter(im(:,:,i));
    % for j = min(min(seg))+1:max(max(seg))
    j = max(max(seg));
    bw = seg==j;
    bw = bwareafilt(bw,[0.00005*numel(bw) .01*numel(bw)]);
    [L num] = bwlabel(bw);
    props = regionprops(L, 'Eccentricity', 'Centroid','Area','ConvexArea');
    for k = 1:num
        if props(k).Eccentricity >=.7 || props(k).Area/props(k).ConvexArea <.8
            L(L==k)=0;
        end
    end
    % end
end


end

function seg = channelSegmenter(im)
filt = fspecial('average',[11 11]);
avg = imfilter(im, filt);
th = multithresh(avg,3);
seg = imquantize(avg,th);
seg = imclose(seg, strel('disk',20));
end

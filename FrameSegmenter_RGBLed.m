function Centroid = FrameSegmenter_RGBLed(im, expected)
im = mat2gray(imread(im));
im = -im(:,:,1)+im(:,:,2)+im(:,:,3);
% red = im(:,:,1);
% green = im(:,:,2);
% blue = im(:,:,3);
s = size(im);
BW = logical(0*im);
seg = channelSegmenter(im);%(:,:,i));
%expected = 4;
for j = min(min(seg))+1:max(max(seg))
    bw = seg==j;
    bw = bwareafilt(bw,[0.00005*numel(bw) .01*numel(bw)]);
    [L num] = bwlabel(bw);
    props = regionprops(L, 'Eccentricity', 'Centroid','Area','ConvexArea');
    for k = 1:num
        %get rid of unwanted slop
        if props(k).Eccentricity >=.5 || props(k).Area/props(k).ConvexArea <.8
            L(L==k)=0;
        end
    end
    BW = logical(min(1, BW+L));
end
[L num] = bwlabel(BW);
count = num;
while(num > count - (expected-1)) && (count > expected)
    BW = imdilate(BW, strel('disk', 1));
    [L num] = bwlabel(BW);        
    % solution has been found
end
props = regionprops(BW, 'Eccentricity','Centroid','Area');
m = find([props.Eccentricity]==max([props.Eccentricity]));
Centroid = props(m).Centroid;
imagesc(BW);hold on
plot(Centroid(1),Centroid(2), '+r'); hold off
end

function seg = channelSegmenter(im)
s = size(im);
kx = 0.0052*s(1);
ky = 0.0052*s(2);
ksx = 2.*round((kx+1)/2)-1;
ksy = 2.*round((ky+1)/2)-1;
filt = fspecial('average',[ksy ksx]);
im = medfilt2(im, [ksy ksx]);
avg = imfilter(im, filt);
th = multithresh(avg,3);
seg = imquantize(avg,th);
seg = imdilate(seg, strel('disk',floor(1.8e-05*s(1)*s(2))));
end

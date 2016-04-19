%% Parameters: image file location, expected number of 'dots' in pattern
% function Centroid = FrameSegmenter_RGBLed(imp, [pattern])
function Centroid = FrameSegmenter_RGBLed(imp, pattern)
expected = size(pattern);
expected = expected(2);

close all
% read image, then calculate g+b+1-r image (easiet for finding dots)
img = mat2gray(imread(imp));
red = img(:,:,1);
green = img(:,:,2);
blue = img(:,:,3);
im = mat2gray((1-img(:,:,1))+img(:,:,2)+img(:,:,3));%idk, i need to find something better
zim = logical(0*im);
BW=zim;
seg = channelSegmenter(im);

% Loop through each of layer of the trinomial image and remove non circular
% components
for j = min(min(seg))+1:max(max(seg))
    bw = seg==j;
    bw = bwareafilt(bw,[0.00005*numel(bw) .01*numel(bw)]);
    [L num] = bwlabel(bw);
    MaskProps = regionprops(L, 'Eccentricity','Area','ConvexArea');
    for i = 1:num
        if MaskProps(i).Eccentricity >=.5 || MaskProps(i).Area/MaskProps(i).ConvexArea <.8
            L(L==i)=0; %remove components
        end
    end
    BW = logical(min(1, BW+L));
end

% Dots are close together, dilate till they touch until an eccentric object
% masks the desired pattern
CC = bwconncomp(BW);
BwMask = BW;
count = CC.NumObjects;
num = count;
while(num >= count - (expected-1)) && (count > expected)
    BwMask = imdilate(BwMask, strel('disk', 2));% higher values are quicker but less resolute
    CCM = bwconncomp(BwMask);
    num = CCM.NumObjects;
    % If/When the number of objects decrease to starting count-number of
    % expected objects-1 (3 objects becomes 1 -> expected-1)
    if (num <= count - (expected-1))
        MaskProps = regionprops(BwMask, 'Eccentricity','Centroid','Area','PixelIdx');
        % Looping through each mask object, checking if it covers pattern
        for j=1:numel(MaskProps)
            if MaskProps(j).Eccentricity < (1-1/expected)
                continue
            end
            prod = zim;
            prod([MaskProps(j).PixelIdxList])=1;
            [L, nnum] = bwlabel(prod.*BW);
            if nnum ~= expected
                continue;
            end;
            % Check for color pattern
            for k = 1:nnum
                color = [sum(sum(red(L==k))); sum(sum(green(L==k))); sum(sum(blue(L==k)))];
                color = round(mat2gray(color))
                if ~isequal(pattern(:,k), color)
                    break;
                elseif k == expected
                        Centroid = MaskProps(j).Centroid;
%                         figure
%                         imagesc(BWD);hold on
%                         plot(Centroid(1),Centroid(2), '+r','linewidth',3); hold off
                        return;
                end;
            end;
        end;
    end;
    % Pattern was not found, continue dilation to join more dots and create
    % a bigger mask
    num = count - (expected-1);
end
end%timeout?


%creates trinay image (dark, medium, birght) -> (1,2,3)
function seg = channelSegmenter(im)
% kernal size calculations
s = size(im);
kx = 0.0052*s(1);
ky = 0.0052*s(2);
ksx = 2.*round((kx+1)/2)-1;
ksy = 2.*round((ky+1)/2)-1;
% Image filtration, median filt then average filt
filt = fspecial('average',[ksy ksx]);
im = medfilt2(im, [ksy ksx]);
avg = imfilter(im, filt);
% Create trinary image
th = multithresh(avg,3);
seg = imquantize(avg,th);
seg = imdilate(seg, strel('disk',floor(1.8e-05*s(1)*s(2))));
end

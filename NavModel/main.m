% Translate an [XYZ] position of the target into a XY in the two fields
% held by the drone.
% 
% Throughout, Camera 0 refers to the downward-facing camera, and camera 1
% refers to the forward-facing camera, as was designed for the original
% drone. 

clear;
clc;

% These values are based  on the hardware drone.
camera0 = Camera;
camera0.vAimAngle = -35;
camera0.positionOffset = [0 20 .5];
camera0.id = 0;

camera1 = Camera;
camera1.vAimAngle = -80;
camera1.positionOffset = [0 -2 -2];
camera1.id = 1;

% Assign a target position. 
Camera.targetPosition([10, 60, -30]);

%% position 1
dronePosition = [0 0 0];

[x0, y0] = camera0.getDotLocation(dronePosition, 1);
[x1, y1] = camera1.getDotLocation(dronePosition, 2);

Camera.plotSpatial(dronePosition, 3, [camera0 camera1]);

keyboard;


%% position 2
dronePosition = [6 15 -10];

[x0, y0] = camera0.getDotLocation(dronePosition, 1);
[x1, y1] = camera1.getDotLocation(dronePosition, 2);

Camera.plotSpatial(dronePosition, 3, [camera0 camera1]);

keyboard;

%% position 3
dronePosition = [9 55 -10];

[x0, y0] = camera0.getDotLocation(dronePosition, 1);
[x1, y1] = camera1.getDotLocation(dronePosition, 2);

Camera.plotSpatial(dronePosition, 3, [camera0 camera1]);


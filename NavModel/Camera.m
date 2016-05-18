classdef Camera
    %Camera A class for a Camera Object.  
    %   Among the properties are the definitions of the view, such as the
    %   viewing angle of the camera, as well as the vertical alignment of
    %   the camera.  All angles are in degrees.  
    %   
    %   In 3-space, all units are inches.
    %       X is positive to the right of the drone.
    %       Y is positive to the fore of the drone.
    %       Z is positive "up" of the drone.
    %       Theta lies in the yz plane, and is relative to the y-axis.
    %       Phi lies in the xy plane, and is relative to the y-axis.
    %       
    %       The aim angle is a theta angle. It is assumed that all cameras 
    %       are aligned along the plane of x=0. 
    %       
    %       All values are approximations of the 
    
    properties
        vViewAngle = 40;
        vResolution = 480
        hViewAngle = 60;
        hResolution = 620
        vAimAngle
        positionOffset = [0,0,0]
        id
    end
    
    methods
        function pCamera = getPCamera(this, pDrone)
            pCamera = pDrone + this.positionOffset;
        end
        
        function aimVector = getAimVector(this)
            aimVector = [0, cosd(this.vAimAngle), sind(this.vAimAngle)]; % this construction makes it inherently normalized.
        end
        
        function targetVector = getTargetVector(this, pDrone)
            targetVector = this.targetPosition() - this.getPCamera(pDrone);
            targetVector = targetVector/norm(targetVector); % normalizing
        end
        
        function [dX, dY] = getDotLocation(this, pDrone, fig)
            % variable prefixes:
            %   v - View or Vertical
            %   h - Horizontal
            %   p - Position
            %   d - Display (*not* delta or derivative thingies!)
            %   c - veCtor
            
            pTarget = this.targetPosition();
            
            % it's a bummer I don't remember linear alhebra, 'cause I'm
            % pretty sure there's an easier way to do this...
            targetVector = this.getTargetVector(pDrone);
            thetaToTarget = acosd(dot(...
                targetVector.*[0,1,1]/norm(targetVector.*[0,1,1]),...
                [0,1,0]));
            if (pDrone(3) > pTarget(3)) 
                thetaToTarget = -thetaToTarget;
            end
            phiToTarget = acosd(dot(...
                targetVector.*[1,1,0]/norm(targetVector.*[1,1,0]),...
                [0,1,0]));
            if (pDrone(1) > pTarget(1)) 
                phiToTarget = -phiToTarget;
            end
            
            vTheta = thetaToTarget - this.vAimAngle;
            
            if (abs(vTheta) > this.vViewAngle/2 || ...
                abs(phiToTarget) > this.hViewAngle/2)
                dX = -1;
                dY = -1;
            else
                hProjectionDist = this.hResolution / (2 * tand(this.hViewAngle / 2));
                vProjectionDist = this.vResolution / (2 * tand(this.vViewAngle / 2));
                
                dX = hProjectionDist * tand(phiToTarget);
                dY = vProjectionDist * tand(vTheta);
                
                dX = dX + this.hResolution / 2;
                dY = -dY + this.vResolution / 2;
                
                dX = round(dX);
                dY = round(dY);
            end
            
            if (nargin > 2 && ~isempty(fig))
                figure(fig);
                clf;
                plot(dX, dY, '*r');
                axis image;
                axis([0 this.hResolution 0 this.vResolution]);
                grid on;
                set(gca,'YDir','reverse');
                title(horzcat('Image from Camera ', num2str(this.id)));
            end
        end
    end
    
    methods(Static)
        function pos = targetPosition(newTargetPosition)
            persistent pTarget;
            if (nargin < 1 && isempty(pTarget))
                error('Target Position is not defined.');
            end
            if (nargin > 0)
                pTarget = newTargetPosition;
            end
            pos = pTarget;
        end
        
        function plotSpatial(pDrone, fig, cameras)
            figure(fig);
            clf;
            hold on;
            t = linspace(0,2*pi,9);
            scatter3([pDrone(1), pDrone(1) + sin(t).*19], ...
                [pDrone(2), pDrone(2) + cos(t).*19], ...
                [pDrone(3), pDrone(3).*ones(1,9)], '*k');
            for cam = cameras
                aimVector = cam.getAimVector(); 
                targetVector = cam.getTargetVector(pDrone);
                pTarget = Camera.targetPosition();
                pCamera = cam.getPCamera(pDrone);
                
                scatter3(pTarget(1), ...
                    pTarget(2), ...
                    pTarget(3), ...
                    '*r');
                quiver3(pCamera(1), ...
                    pCamera(2), ...
                    pCamera(3), ...
                    aimVector(1).*10, ...
                    aimVector(2).*10, ...
                    aimVector(3).*10, ...
                    '-b');
                quiver3(pCamera(1), ...
                    pCamera(2), ...
                    pCamera(3), ...
                    targetVector(1).*10, ...
                    targetVector(2).*10, ...
                    targetVector(3).*10, ...
                    '-r');
            end           


            hold off;
            grid on;
            axis image;
            set(gca, 'DataAspectRatio', [1,1,1]);
            xlabel('x');
            ylabel('y');
            zlabel('z');
            view(45, 45);
            legend( ...
                'Drone', ...
                'Target', ...
                'Camera View Center', ...
                'Cam-to-Target Vector');
        end
    end
    
end


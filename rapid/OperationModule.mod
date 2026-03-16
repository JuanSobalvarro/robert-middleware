MODULE OperationModule
    VAR robtarget destination;
    
    PROC ExecuteAction(string action_cmd, \robtarget target_coords, \robtarget circular_extra_target, \jointtarget joint_target, \num speed, \zonedata zone)
        TPWrite "Action executed at time: " + CTime();
        
        TEST action_cmd
            CASE "PING":
                SendResponse "ACK|PONGUWU";
            CASE "MoveL":
                IF Present(target_coords) THEN
                    TPWrite "Action: MoveL to : " + ValToStr(target_coords);
                    MoveL target_coords, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveL.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveL";
                
            CASE "MoveJ":
                IF Present(target_coords) THEN
                    TPWrite "Action: MoveJ to : " + ValToStr(target_coords);
                    MoveJ target_coords, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveJ.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveJ";

            CASE "MoveAbsJ":
                IF Present(joint_target) THEN
                    TPWrite "Action: MoveAbsJ to : " + ValToStr(joint_target);
                    MoveAbsJ joint_target, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveAbsJ.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveAbsJ";
            
            CASE "MoveC":
                IF Present(target_coords) AND Present(circular_extra_target) THEN
                    TPWrite "Action: MoveC to : " + ValToStr(target_coords) + " via " + ValToStr(circular_extra_target);

                    MoveC circular_extra_target, target_coords, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveC.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveC";

            CASE "SetSpeed":
                IF Present(joint_target) THEN 
                    TPWrite "Action: Setting speed to : " + NumToStr(speed, 0) + " mm/s";
                    move_speed.v_tcp := speed;
                ELSE
                    TPWrite "Error: Missing speed value for SetSpeed.";
                    SendResponse "NACK|MISSING_SPEED";
                    RETURN;
                ENDIF

                SendResponse "ACK|SetSpeed";

            CASE "SetZone":
                TPWrite "Action: Setting zone...";
                ! Implementation for setting zone
                SendResponse "ACK|SetZone";

            CASE "ORIGIN":
                TPWrite "Action: Moving to origin...";
                
                MoveJ origin, move_speed, move_zone, tool0; 

                SendResponse "ACK|ORIGIN";
                
            CASE "HOME":
                TPWrite "Action: Executing Home sequence...";
                ! A standard MoveAbsJ is best for homing, bypassing singularity checks
                MoveAbsJ ZERO, move_speed, move_zone, tool0;
                
                SendResponse "ACK|HOME";

            ! unknown command
            DEFAULT:
                TPWrite "Error: Operation not demove_zoned for command: " + action_cmd;

                SendResponse "NACK|UNKNOWN_COMMAND";
        ENDTEST
        
    ENDPROC

ENDMODULE
MODULE OperationModule
    VAR robtarget destination;

    PROC ExecuteAction(string action_cmd, \robtarget target_coords, \robtarget circular_extra_target, \jointtarget joint_target, \num speed, \zonedata zone)
        TPWrite "Action executed at time: " + CTime();

        TEST action_cmd
            CASE "PING":
                SendResponse "NACK|This ping should be answered at the middleware level, not here!";
            CASE "PINGR":
                SendResponse "ACK|PONGRUWU";
            CASE "MoveL":
                IF Present(target_coords) THEN
                    MoveL target_coords, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveL.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveL";

            CASE "MoveJ":
                IF Present(target_coords) THEN
                    MoveJ target_coords, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveJ.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveJ";

            CASE "MoveAbsJ":
                IF Present(joint_target) THEN
                    MoveAbsJ joint_target, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveAbsJ.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveAbsJ";

            CASE "MoveC":
                IF Present(target_coords) AND Present(circular_extra_target) THEN

                    MoveC target_coords, circular_extra_target, move_speed, move_zone, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveC.";
                    SendResponse "NACK|MISSING_COORDS";
                    RETURN;
                ENDIF

                SendResponse "ACK|MoveC";

            CASE "SetSpeed":
                IF Present(speed) THEN
                    IF speed < 5 OR speed > MaxRobSpeed() THEN
                        TPWrite "Error: Speed value out of range (5 - " + ValToStr(MaxRobSpeed()) + "). Received: " + ValToStr(speed);
                        SendResponse "NACK|SPEED_OUT_OF_RANGE";
                        RETURN;
                    ENDIF

                    TPWrite "Action: Setting speed to : " + NumToStr(speed, 0) + " mm/s";
                    move_speed.v_tcp := speed;

                    SendResponse "ACK|SetSpeed";
                    RETURN;
                ELSE
                    TPWrite "Error: Missing speed value for SetSpeed.";
                    SendResponse "NACK|MISSING_SPEED";
                    RETURN;
                ENDIF

            CASE "SetZone":
                TPWrite "Action: Setting zone...";

                move_zone := zone;

                SendResponse "ACK|SetZone";

            CASE "ZERO":
                TPWrite "Action: Moving to zero position...";
                MoveAbsJ ZERO, move_speed, move_zone, tool0;
                SendResponse "ACK|ZERO";

            CASE "GETSTATUS":
                TPWrite "Action: Getting robot status...";
                HandleGetState;

            ! unknown command
            DEFAULT:
                TPWrite "Error: Operation not demove_zoned for command: " + action_cmd;

                SendResponse "NACK|UNKNOWN_COMMAND";
        ENDTEST

    ENDPROC

ENDMODULE

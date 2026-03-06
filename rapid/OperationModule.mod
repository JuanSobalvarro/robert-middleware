MODULE OperationModule
    VAR robtarget destination;
    
    PROC ExecuteAction(string action_cmd, \pos target_coords)
        TPWrite "Action executed at time: " + CTime();
        
        TEST action_cmd
            CASE "MOVEJ":
                IF Present(target_coords) THEN
                    TPWrite "Action: Moving to -> X:" + NumToStr(target_coords.x, 2) + " Y:" + NumToStr(target_coords.y, 2) + " Z:" + NumToStr(target_coords.z, 2);
                    
                    destination := CRobT(\Tool:=tool0 \WObj:=wobj0);
                    
                    destination.trans := target_coords;
                    
                    MoveJ destination, v100, fine, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MOVEJ command.";
                ENDIF
                
            CASE "ORIGIN":
                TPWrite "Action: Moving to origin...";
                
                MoveJ origin, v1000, fine, tool0; 
                
            CASE "HOME":
                TPWrite "Action: Executing Home sequence...";
                ! A standard MoveAbsJ is best for homing, bypassing singularity checks
                MoveAbsJ ZERO, v100, fine, tool0;
                
            DEFAULT:
                TPWrite "Error: Operation not defined for command: " + action_cmd;
        ENDTEST
        
    ENDPROC

ENDMODULE
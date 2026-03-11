MODULE OperationModule
    VAR robtarget destination;
    VAR jointtarget jt1;
    
    PROC ExecuteAction(string action_cmd, \robtarget target_coords, \robtarget circular_extra_target)
        TPWrite "Action executed at time: " + CTime();
        
        TEST action_cmd
            CASE "MJ":
                IF Present(target_coords) THEN
                    TPWrite "Action: MoveJ to : " + ValToStr(target_coords);
                    MoveJ target_coords, v100, fine, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveJ.";
                ENDIF
            
            CASE "ML":
                IF Present(target_coords) THEN
                    TPWrite "Action: MoveL to : " + ValToStr(target_coords);
                    MoveL target_coords, v100, fine, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveL.";
                ENDIF

            CASE "MA":
                IF Present(target_coords) THEN
                    TPWrite "Action: MoveAbsJ to : " + ValToStr(target_coords);
                    jt1 := CalcJointT(target_coords, tool0 \WObj:=wobj0);
                    MoveAbsJ jt1, v100, fine, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveAbsJ.";
                ENDIF
            
            CASE "MC":
                IF Present(target_coords) AND Present(circular_extra_target) THEN
                    TPWrite "Action: MoveC to : " + ValToStr(target_coords) + " via " + ValToStr(circular_extra_target);

                    MoveC circular_extra_target, target_coords, v100, fine, tool0;
                ELSE
                    TPWrite "Error: Missing coordinates for MoveC.";
                ENDIF

            CASE "ORG":
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
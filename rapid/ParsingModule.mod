MODULE ParsingModule
    
    PROC parse_message(string message)
        VAR num comma1;
        VAR num comma2;
        VAR num comma3;
        
        VAR string cmd;
        VAR string str_x;
        VAR string str_y;
        VAR string str_z;
        
        VAR pos target_pos; 
        VAR bool conversion_ok;
        
        comma1 := StrFind(message, 1, ",");
        
        IF comma1 < StrLen(message) THEN
            cmd := StrPart(message, 1, comma1 - 1);
            
            IF cmd = "MOVEJ" THEN
                comma2 := StrFind(message, comma1 + 1, ",");
                comma3 := StrFind(message, comma2 + 1, ",");
                
                str_x := StrPart(message, comma1 + 1, (comma2 - comma1) - 1);
                str_y := StrPart(message, comma2 + 1, (comma3 - comma2) - 1);
                str_z := StrPart(message, comma3 + 1, StrLen(message) - comma3);
                
                conversion_ok := StrToVal(str_x, target_pos.x);
                conversion_ok := StrToVal(str_y, target_pos.y);
                conversion_ok := StrToVal(str_z, target_pos.z);
                
                ExecuteAction cmd \target_coords:=target_pos;
            ELSE
                TPWrite "Unknown comma-separated command: " + cmd;
            ENDIF
            
        ELSE
            cmd := message;
            
            IF cmd = "HOME" THEN
                ExecuteAction cmd;
            ELSEIF cmd = "ORIGIN" THEN
                ExecuteAction cmd;
            ELSEIF cmd <> "EXIT" THEN
                TPWrite "Unknown single command: " + cmd;
            ENDIF
        ENDIF
    ENDPROC

ENDMODULE
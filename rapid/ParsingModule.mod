MODULE ParsingModule
    ! Helper variable to reconstruct fragmented strings
    VAR string full_payload;

    PROC parse_message(string parts{*})
        VAR string cmd;
        VAR num sep1;
        VAR num sep2;
        VAR num sep3;
        VAR robtarget target1;
        VAR robtarget target2;
        VAR bool conversion_ok;
        
        ! Identify Command (usually in the first 10 chars)
        sep1 := StrFind(parts{1}, 1, "|");
        
        IF sep1 > 0 THEN
            cmd := StrPart(parts{1}, 1, sep1 - 1);
            
            TEST cmd
                CASE "ML", "MJ", "MA":
                    full_payload := GetFragment(parts, sep1 + 1, ";");
                    
                    IF StrToVal(full_payload, target1) THEN
                        ExecuteAction cmd \target_coords:=target1;
                    ELSE
                        TPWrite "Error: StrToVal failed for " + cmd;
                    ENDIF

                CASE "MC":
                    ! Format: MC|CIR_TARGET|DEST_TARGET;
                    sep2 := FindInParts(parts, sep1 + 1, "|");
                    
                    ! Extract CIR_TARGET
                    full_payload := GetSubFragment(parts, sep1 + 1, sep2 - 1);
                    conversion_ok := StrToVal(full_payload, target1);
                    
                    ! Extract DEST_TARGET
                    sep3 := FindInParts(parts, sep2 + 1, ";");
                    full_payload := GetSubFragment(parts, sep2 + 1, sep3 - 1);
                    conversion_ok := conversion_ok AND StrToVal(full_payload, target2);

                    IF conversion_ok THEN
                        ExecuteAction "MC" \target_coords:=target2, \circular_extra_target:=target1;
                    ELSE
                        TPWrite "Error: StrToVal failed for MoveC";
                    ENDIF

                CASE "HOME", "ORG":
                    ExecuteAction cmd;

                DEFAULT:
                    TPWrite "Unknown command: " + cmd;
            ENDTEST
        ELSE
            TPWrite "Format Error: No | found in first 80 chars";
        ENDIF
    ENDPROC

    FUNC robtarget retrieve_robtarget(string parts{*}, num offset_cmd)
        ! parts will be something like:
        ! "MJ|[400.00,0.00,500.00][0.000000,0.000000,-1.000000,0.000000][0,0,-1,0][9e9," "9e9,9e9,9e9,9e9,9e9]"
        ! now offset cmd is the index where the "MJ|" part ends, so we can start looking for the robtarget from there
        VAR robtarget result;
        VAR string pos_str;
        VAR string orient_str;
        VAR string conf_str;

        VAR num sep_coords;
        VAR num sep_orient;
        VAR num sep_conf;

        VAR string temp;

        ! Extract position
        sep_coord := find_in_parts(parts, offset_cmd, "]");

        IF sep_coord > 0 THEN;
            temp := get_substring_from_parts(parts, offset_cmd, sep_coord);
        ELSE


    ENDFUNC

    FUNC num find_in_parts(string parts{*}, num start_index, string delimiter)
        VAR num i;
        VAR num sep_pos;
        FOR i FROM start_index TO LEN(parts) DO
            sep_pos := StrFind(parts{i}, 1, delimiter);
            IF sep_pos > 0 THEN
                RETURN i;
            ENDIF
        ENDFOR
        RETURN -1; ! Not found
    ENDFUNC

    FUNC string get_substring_from_parts(string parts{*}, num start_index, num end_index)
        VAR string result;
        VAR num i;
        
        ! look into parts until the start index is reached, then concatenate until the end index is reached
        ! remembering that parts is an array of strings like: "uwuwuw" "a,jsd" "uwan,sa"
        ! so if we want to get substring between commas (which we already calculated indexes) we will need to navigate through each
        ! character and add them only when we are between the start and end index
        VAR num char_count := 0;
        FOR i FROM 1 TO LEN(parts) DO
            ! todo 

        RETURN result;

ENDMODULE
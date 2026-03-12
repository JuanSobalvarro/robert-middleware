MODULE ParsingModule
    ! Buffer structure for incoming messages
    ! command_id (byte 1)
    ! target.x (bytes 2-9)
    ! target.y (bytes 10-17)
    ! target.z (bytes 18-25)
    ! target.rx (bytes 26-33)
    ! target2.x (bytes 34-41)
    ! target2.y (bytes 42-49)
    ! target2.z (bytes 50-57)
    ! target2.rx (bytes 58-65)
    ! target2.ry (bytes 66-73)
    ! target2.rz (bytes 74-81)
    ! ... y asi xd
    VAR num command_id;
    VAR string command_str;
    VAR robtarget parsed_target;
    VAR robtarget parsed_target2;
    VAR jointtarget parsed_joint_target;
    VAR num speed;
    VAR num zone_id;

    PROC parse_message(VAR rawbytes raw_buffer, num len)

        IF len < 4 THEN
            TPWrite "Error: Received message is too short to contain a valid command.";
            RETURN;
        ENDIF

        ! command id set
        UnpackRawBytes raw_buffer, 1, command_id \IntX := USINT;

        command_str := command_to_string(command_id);

        IF NOT is_move_command(command_str) THEN
            ExecuteAction command_str;
            RETURN;
        ENDIF

        IF command_str = "SetSpeed" THEN
            UnpackRawBytes raw_buffer, 180, speed \Float4;
            ExecuteAction command_str, \speed := speed;
        ENDIF

        IF command_str = "SetZone" THEN
            UnpackRawBytes raw_buffer, 184, zone_id \IntX := USINT;
            ExecuteAction command_str, \zone := value_to_zone(zone_id);
            RETURN;
        ENDIF

        TPWrite "[PARSING] Command received: " + command_str;

        ! robtarget position
        UnpackRawBytes raw_buffer, 2, parsed_target.trans.x \Float4;
        UnpackRawBytes raw_buffer, 6, parsed_target.trans.y \Float4;
        UnpackRawBytes raw_buffer, 10, parsed_target.trans.z \Float4;

        TPWrite "[PARSING] Position: " + ValToStr(parsed_target.trans);
        
        ! robtarget orientation
        UnpackRawBytes raw_buffer, 14, parsed_target.rot.q1 \Float4;
        UnpackRawBytes raw_buffer, 18, parsed_target.rot.q2 \Float4;
        UnpackRawBytes raw_buffer, 22, parsed_target.rot.q3 \Float4;
        UnpackRawBytes raw_buffer, 26, parsed_target.rot.q4 \Float4;

        ! robtarget configuration 
        UnpackRawBytes raw_buffer, 30, parsed_target.robconf.cf1 \IntX := DINT;
        UnpackRawBytes raw_buffer, 34, parsed_target.robconf.cf4 \IntX := DINT;
        UnpackRawBytes raw_buffer, 38, parsed_target.robconf.cf6 \IntX := DINT;
        UnpackRawBytes raw_buffer, 42, parsed_target.robconf.cfx \IntX := USINT;

        ! robtarget joints
        UnpackRawBytes raw_buffer, 43, parsed_target.extax.eax_a \Float4;
        UnpackRawBytes raw_buffer, 47, parsed_target.extax.eax_b \Float4;
        UnpackRawBytes raw_buffer, 51, parsed_target.extax.eax_c \Float4;
        UnpackRawBytes raw_buffer, 55, parsed_target.extax.eax_d \Float4;
        UnpackRawBytes raw_buffer, 59, parsed_target.extax.eax_e \Float4;
        UnpackRawBytes raw_buffer, 63, parsed_target.extax.eax_f \Float4;

        IF command_str = "MoveJ" OR command_str = "MoveL" THEN
            ! For MoveJ, we also need to parse the second target for the via point
            ExecuteAction command_str, \target_coords := parsed_target;
            RETURN;
        ENDIF

        ! robtarget 2 for Move C
        UnpackRawBytes raw_buffer, 67, parsed_target2.trans.x \Float4;
        UnpackRawBytes raw_buffer, 71, parsed_target2.trans.y \Float4;
        UnpackRawBytes raw_buffer, 75, parsed_target2.trans.z \Float4;
        UnpackRawBytes raw_buffer, 79, parsed_target2.rot.q1 \Float4;
        UnpackRawBytes raw_buffer, 83, parsed_target2.rot.q2 \Float4;
        UnpackRawBytes raw_buffer, 87, parsed_target2.rot.q3 \Float4;
        UnpackRawBytes raw_buffer, 91, parsed_target2.rot.q4 \Float4;
        UnpackRawBytes raw_buffer, 95, parsed_target2.robconf.cf1 \IntX := DINT;
        UnpackRawBytes raw_buffer, 99, parsed_target2.robconf.cf4 \IntX := DINT;
        UnpackRawBytes raw_buffer, 103, parsed_target2.robconf.cf6 \IntX := DINT;
        UnpackRawBytes raw_buffer, 107, parsed_target2.robconf.cfx \IntX := USINT;
        UnpackRawBytes raw_buffer, 108, parsed_target2.extax.eax_a \Float4;
        UnpackRawBytes raw_buffer, 112, parsed_target2.extax.eax_b \Float4;
        UnpackRawBytes raw_buffer, 116, parsed_target2.extax.eax_c \Float4;
        UnpackRawBytes raw_buffer, 120, parsed_target2.extax.eax_d \Float4;
        UnpackRawBytes raw_buffer, 124, parsed_target2.extax.eax_e \Float4;
        UnpackRawBytes raw_buffer, 128, parsed_target2.extax.eax_f \Float4;

        IF command_str = "MoveC" THEN
            ExecuteAction command_str, \target_coords := parsed_target, \circular_extra_target := parsed_target2;
            RETURN;
        ENDIF

        ! now it only remains MoveAbsJ which has a jointtarget and a external joints
        UnpackRawBytes raw_buffer, 132, parsed_joint_target.robax.rax_1 \Float4;
        UnpackRawBytes raw_buffer, 136, parsed_joint_target.robax.rax_2 \Float4;
        UnpackRawBytes raw_buffer, 140, parsed_joint_target.robax.rax_3 \Float4;
        UnpackRawBytes raw_buffer, 144, parsed_joint_target.robax.rax_4 \Float4;
        UnpackRawBytes raw_buffer, 148, parsed_joint_target.robax.rax_5 \Float4;
        UnpackRawBytes raw_buffer, 152, parsed_joint_target.robax.rax_6 \Float4;
        UnpackRawBytes raw_buffer, 156, parsed_joint_target.extax.eax_a \Float4;
        UnpackRawBytes raw_buffer, 160, parsed_joint_target.extax.eax_b \Float4;
        UnpackRawBytes raw_buffer, 164, parsed_joint_target.extax.eax_c \Float4;
        UnpackRawBytes raw_buffer, 168, parsed_joint_target.extax.eax_d \Float4;
        UnpackRawBytes raw_buffer, 172, parsed_joint_target.extax.eax_e \Float4;
        UnpackRawBytes raw_buffer, 176, parsed_joint_target.extax.eax_f \Float4;

        ExecuteAction command_str, \joint_target := parsed_joint_target;

    ENDPROC

    FUNC bool is_move_command(string cmd)
        IF cmd = "MoveJ" OR cmd = "MoveAbsJ" OR cmd = "MoveC" OR cmd = "MoveL" THEN
            RETURN TRUE;
        ENDIF

        RETURN FALSE;
    ENDFUNC
        
    FUNC string command_to_string(num cmd)
        TEST cmd
            CASE 0x00:
                RETURN "MoveL";
            CASE 0x01:
                RETURN "MoveJ";
            CASE 0x02:
                RETURN "MoveC";
            CASE 0x03:
                RETURN "MoveAbsJ";
            CASE 0x04:
                RETURN "SetSpeed";
            CASE 0x05:
                RETURN "SetZone";
            CASE 0x06:
                RETURN "Exit";
            DEFAULT:
                RETURN "UNKNOWN";
        ENDTEST
    ENDFUNC
ENDMODULE
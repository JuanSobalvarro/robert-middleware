MODULE ParsingModule
    ! Buffer structure for incoming messages
    ! command_id (byte 1)
    ! target.x (bytes 2-9)
    ! target.y (bytes 10-17)
    ! target.z (bytes 18-25)
    ! target.rx (bytes 26-33)
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

        TPWrite "[PARSING] Received command ID: " + ValToStr(command_id) + " -> " + command_str;

        IF command_str = "GETSTATUS" THEN
            ExecuteAction command_str;
            RETURN;
        ENDIF

        IF command_str = "SetZone" THEN
            UnpackRawBytes raw_buffer, 2, zone_id \IntX := USINT;
            TPWrite "[PARSING] Zone ID: " + ValToStr(zone_id);
            ExecuteAction command_str, \zone := value_to_zone(zone_id);
            RETURN;
        ENDIF

        IF command_str = "SetSpeed" THEN
            UnpackRawBytes raw_buffer, 3, speed \IntX := UINT;
            TPWrite "[PARSING] Speed: " + ValToStr(speed);
            ExecuteAction command_str, \speed := speed;
            RETURN;
        ENDIF

        IF NOT is_move_command(command_str) THEN
            ExecuteAction command_str;
            RETURN;
        ENDIF

        TPWrite "[PARSING] Command received: " + command_str;

        ! robtarget position
        UnpackRawBytes raw_buffer, 5, parsed_target.trans.x \Float4;
        UnpackRawBytes raw_buffer, 9, parsed_target.trans.y \Float4;
        UnpackRawBytes raw_buffer, 13, parsed_target.trans.z \Float4;

        TPWrite "[PARSING] Position: " + ValToStr(parsed_target.trans);
        
        ! robtarget orientation
        UnpackRawBytes raw_buffer, 17, parsed_target.rot.q1 \Float4;
        UnpackRawBytes raw_buffer, 21, parsed_target.rot.q2 \Float4;
        UnpackRawBytes raw_buffer, 25, parsed_target.rot.q3 \Float4;
        UnpackRawBytes raw_buffer, 29, parsed_target.rot.q4 \Float4;

        ! robtarget configuration 
        UnpackRawBytes raw_buffer, 33, parsed_target.robconf.cf1 \IntX := DINT;
        UnpackRawBytes raw_buffer, 37, parsed_target.robconf.cf4 \IntX := DINT;
        UnpackRawBytes raw_buffer, 41, parsed_target.robconf.cf6 \IntX := DINT;
        UnpackRawBytes raw_buffer, 45, parsed_target.robconf.cfx \IntX := UDINT;

        ! robtarget joints
        UnpackRawBytes raw_buffer, 49, parsed_target.extax.eax_a \Float4;
        UnpackRawBytes raw_buffer, 53, parsed_target.extax.eax_b \Float4;
        UnpackRawBytes raw_buffer, 57, parsed_target.extax.eax_c \Float4;
        UnpackRawBytes raw_buffer, 61, parsed_target.extax.eax_d \Float4;
        UnpackRawBytes raw_buffer, 65, parsed_target.extax.eax_e \Float4;
        UnpackRawBytes raw_buffer, 69, parsed_target.extax.eax_f \Float4;

        IF command_str = "MoveJ" OR command_str = "MoveL" THEN
            ! For MoveJ, we also need to parse the second target for the via point
            ExecuteAction command_str, \target_coords := parsed_target;
            RETURN;
        ENDIF

        ! robtarget 2 for Move C

        UnpackRawBytes raw_buffer, 73, parsed_target2.trans.x \Float4;
        UnpackRawBytes raw_buffer, 77, parsed_target2.trans.y \Float4;
        UnpackRawBytes raw_buffer, 81, parsed_target2.trans.z \Float4;

        UnpackRawBytes raw_buffer, 85, parsed_target2.rot.q1 \Float4;
        UnpackRawBytes raw_buffer, 89, parsed_target2.rot.q2 \Float4;
        UnpackRawBytes raw_buffer, 93, parsed_target2.rot.q3 \Float4;
        UnpackRawBytes raw_buffer, 97, parsed_target2.rot.q4 \Float4;

        UnpackRawBytes raw_buffer, 101, parsed_target2.robconf.cf1 \IntX := DINT;
        UnpackRawBytes raw_buffer, 105, parsed_target2.robconf.cf4 \IntX := DINT;
        UnpackRawBytes raw_buffer, 109, parsed_target2.robconf.cf6 \IntX := DINT;
        UnpackRawBytes raw_buffer, 113, parsed_target2.robconf.cfx \IntX := UDINT;

        UnpackRawBytes raw_buffer, 117, parsed_target2.extax.eax_a \Float4;
        UnpackRawBytes raw_buffer, 121, parsed_target2.extax.eax_b \Float4;
        UnpackRawBytes raw_buffer, 125, parsed_target2.extax.eax_c \Float4;
        UnpackRawBytes raw_buffer, 129, parsed_target2.extax.eax_d \Float4;
        UnpackRawBytes raw_buffer, 133, parsed_target2.extax.eax_e \Float4;
        UnpackRawBytes raw_buffer, 137, parsed_target2.extax.eax_f \Float4;

        IF command_str = "MoveC" THEN
            ExecuteAction command_str, \target_coords := parsed_target, \circular_extra_target := parsed_target2;
            RETURN;
        ENDIF

        ! now it only remains MoveAbsJ which has a jointtarget and a external joints
        UnpackRawBytes raw_buffer, 129, parsed_joint_target.robax.rax_1 \Float4;
        UnpackRawBytes raw_buffer, 133, parsed_joint_target.robax.rax_2 \Float4;
        UnpackRawBytes raw_buffer, 137, parsed_joint_target.robax.rax_3 \Float4;
        UnpackRawBytes raw_buffer, 141, parsed_joint_target.robax.rax_4 \Float4;
        UnpackRawBytes raw_buffer, 145, parsed_joint_target.robax.rax_5 \Float4;
        UnpackRawBytes raw_buffer, 149, parsed_joint_target.robax.rax_6 \Float4;
        ! external
        UnpackRawBytes raw_buffer, 153, parsed_joint_target.extax.eax_a \Float4;
        UnpackRawBytes raw_buffer, 157, parsed_joint_target.extax.eax_b \Float4;
        UnpackRawBytes raw_buffer, 161, parsed_joint_target.extax.eax_c \Float4;
        UnpackRawBytes raw_buffer, 165, parsed_joint_target.extax.eax_d \Float4;
        UnpackRawBytes raw_buffer, 169, parsed_joint_target.extax.eax_e \Float4;
        UnpackRawBytes raw_buffer, 173, parsed_joint_target.extax.eax_f \Float4;

        ExecuteAction command_str, \joint_target := parsed_joint_target;

    ENDPROC

    FUNC bool is_move_command(string cmd)
        IF cmd = "MoveJ" OR cmd = "MoveAbsJ" OR cmd = "MoveC" OR cmd = "MoveL" THEN
            RETURN TRUE;
        ENDIF

        RETURN FALSE;
    ENDFUNC
ENDMODULE
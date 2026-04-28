MODULE TelemetryModule
    PROC HandleGetState()
        VAR rawbytes response_buffer;
        VAR robtarget currentTarget;
        VAR jointtarget currentJoints;
        VAR num speedOverride;
        VAR symnum opMode;
        VAR string robotTime;
        VAR string robotDate;

        ClearRawBytes response_buffer;

        currentTarget := CRobT(\Tool:=tool0 \WObj:=wobj0);
        currentJoints := CJointT();
        speedOverride := CSpeedOverride();
        opMode := OpMode();
        robotTime := CTime();
        robotDate := CDate();
    
        ! op mode byte 0
        PackRawBytes opMode, response_buffer, 0 \IntX := USINT;

        ! speed override byte 1 - 4
        PackRawBytes speedOverride, response_buffer, 1 \Float4;
        
        ! current speed
        PackRawBytes move_speed.v_tcp, response_buffer, 5 \Float4;

        ! current zone
        PackRawBytes zone_to_value(move_zone), response_buffer, 9 \Float4;

        PackRawBytes currentTarget.trans.x, response_buffer, 13 \Float4;
        PackRawBytes currentTarget.trans.y, response_buffer, 17 \Float4;
        PackRawBytes currentTarget.trans.z, response_buffer, 21 \Float4;
        
        PackRawBytes currentTarget.rot.q1, response_buffer, 25 \Float4;
        PackRawBytes currentTarget.rot.q2, response_buffer, 29 \Float4;
        PackRawBytes currentTarget.rot.q3, response_buffer, 33 \Float4;
        PackRawBytes currentTarget.rot.q4, response_buffer, 37 \Float4;

        PackRawBytes currentTarget.robconf.cf1, response_buffer, 41 \IntX := DINT;
        PackRawBytes currentTarget.robconf.cf4, response_buffer, 45 \IntX := DINT;
        PackRawBytes currentTarget.robconf.cf6, response_buffer, 49 \IntX := DINT;
        PackRawBytes currentTarget.robconf.cfx, response_buffer, 53 \IntX := DINT;

        PackRawBytes currentTarget.extax.eax_a, response_buffer, 57 \Float4;
        PackRawBytes currentTarget.extax.eax_b, response_buffer, 61 \Float4;
        PackRawBytes currentTarget.extax.eax_c, response_buffer, 65 \Float4;
        PackRawBytes currentTarget.extax.eax_d, response_buffer, 69 \Float4;
        PackRawBytes currentTarget.extax.eax_e, response_buffer, 73 \Float4;
        PackRawBytes currentTarget.extax.eax_f, response_buffer, 77 \Float4;

        PackRawBytes currentJoints.robax.rax_1, response_buffer, 81 \Float4;
        PackRawBytes currentJoints.robax.rax_2, response_buffer, 85 \Float4;
        PackRawBytes currentJoints.robax.rax_3, response_buffer, 89 \Float4;
        PackRawBytes currentJoints.robax.rax_4, response_buffer, 93 \Float4;
        PackRawBytes currentJoints.robax.rax_5, response_buffer, 97 \Float4;
        PackRawBytes currentJoints.robax.rax_6, response_buffer, 101 \Float4;

        PackRawBytes currentJoints.extax.eax_a, response_buffer, 105 \Float4;
        PackRawBytes currentJoints.extax.eax_b, response_buffer, 109 \Float4;
        PackRawBytes currentJoints.extax.eax_c, response_buffer, 113 \Float4;
        PackRawBytes currentJoints.extax.eax_d, response_buffer, 117 \Float4;
        PackRawBytes currentJoints.extax.eax_e, response_buffer, 121 \Float4;
        PackRawBytes currentJoints.extax.eax_f, response_buffer, 125 \Float4;

        PackRawBytes robotTime, response_buffer, 129 \Str;
        PackRawBytes robotDate, response_buffer, 161 \Str;

        SendBinaryResponse response_buffer;
        TPWrite "Action: Sent Robot State (Binary)";
    ENDPROC
ENDMODULE
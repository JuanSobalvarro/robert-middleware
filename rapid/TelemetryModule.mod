MODULE TelemetryModule
    PROC HandleGetState()
        VAR rawbytes response_buffer;
        VAR robtarget currentTarget;
        VAR jointtarget currentJoints;
        VAR num speedOverride;
        VAR num opMode;
        VAR string robotTime;
        VAR string robotDate;

        ClearRawBytes response_buffer;

        currentTarget := CRobT(\Tool:=tool0 \WObj:=wobj0);
        currentJoints := CJointT();
        speedOverride := CSpeedOverride();
        opMode := 0;
        robotTime := CTime();
        robotDate := CDate();
    
        ! op mode byte 0
        PackRawBytes opMode, response_buffer, 1 \IntX := USINT;

        ! speed override byte 1 - 4
        PackRawBytes speedOverride, response_buffer, 2 \Float4;
        
        ! current speed
        PackRawBytes move_speed.v_tcp, response_buffer, 6 \Float4;

        ! current zone
        PackRawBytes zone_to_value(move_zone), response_buffer, 10 \Float4;

        PackRawBytes currentTarget.trans.x, response_buffer, 14 \Float4;
        PackRawBytes currentTarget.trans.y, response_buffer, 18 \Float4;
        PackRawBytes currentTarget.trans.z, response_buffer, 22 \Float4;
        
        PackRawBytes currentTarget.rot.q1, response_buffer, 26 \Float4;
        PackRawBytes currentTarget.rot.q2, response_buffer, 30 \Float4;
        PackRawBytes currentTarget.rot.q3, response_buffer, 34 \Float4;
        PackRawBytes currentTarget.rot.q4, response_buffer, 38 \Float4;

        PackRawBytes currentTarget.robconf.cf1, response_buffer, 42 \IntX := DINT;
        PackRawBytes currentTarget.robconf.cf4, response_buffer, 46 \IntX := DINT;
        PackRawBytes currentTarget.robconf.cf6, response_buffer, 50 \IntX := DINT;
        PackRawBytes currentTarget.robconf.cfx, response_buffer, 54 \IntX := DINT;

        PackRawBytes currentTarget.extax.eax_a, response_buffer, 58 \Float4;
        PackRawBytes currentTarget.extax.eax_b, response_buffer, 62 \Float4;
        PackRawBytes currentTarget.extax.eax_c, response_buffer, 66 \Float4;
        PackRawBytes currentTarget.extax.eax_d, response_buffer, 70 \Float4;
        PackRawBytes currentTarget.extax.eax_e, response_buffer, 74 \Float4;
        PackRawBytes currentTarget.extax.eax_f, response_buffer, 78 \Float4;

        PackRawBytes currentJoints.robax.rax_1, response_buffer, 82 \Float4;
        PackRawBytes currentJoints.robax.rax_2, response_buffer, 86 \Float4;
        PackRawBytes currentJoints.robax.rax_3, response_buffer, 90 \Float4;
        PackRawBytes currentJoints.robax.rax_4, response_buffer, 94 \Float4;
        PackRawBytes currentJoints.robax.rax_5, response_buffer, 98 \Float4;
        PackRawBytes currentJoints.robax.rax_6, response_buffer, 102 \Float4;

        PackRawBytes currentJoints.extax.eax_a, response_buffer, 106 \Float4;
        PackRawBytes currentJoints.extax.eax_b, response_buffer, 110 \Float4;
        PackRawBytes currentJoints.extax.eax_c, response_buffer, 114 \Float4;
        PackRawBytes currentJoints.extax.eax_d, response_buffer, 118 \Float4;
        PackRawBytes currentJoints.extax.eax_e, response_buffer, 122 \Float4;
        PackRawBytes currentJoints.extax.eax_f, response_buffer, 126 \Float4;

        PackRawBytes robotTime, response_buffer, 130 \ASCII;
        PackRawBytes robotDate, response_buffer, 162 \ASCII;

        SendBinaryResponse response_buffer;
        TPWrite "Action: Sent Robot State (Binary)";
    ENDPROC
ENDMODULE
# TODO moves
- MoveJ
- MoveAbsJ
- MoveC
- MoveL

# Windows firewall
```bash
New-NetFirewallRule -DisplayName "RobeRT-RobotStudio-In" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 5000
```

# Take in count
- EtherCat
- ROS2 DDS
- shift from std::stringstream to std::memcpy

# keywords
- Hardware Abstraction Layer (HAL)
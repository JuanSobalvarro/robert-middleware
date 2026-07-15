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

# About architectural decisions
- Why separate rapid data types from proto types?
Simply because we need separation of concerns even though we may have the same data duplicated in both. The rapid structures are for binary data transmission, while the proto structures are for api serialization of messages between a user client and the middleware server. This separation allows us to optimize each for its specific use case without one affecting the other. For example, we can change the proto structures for better API design without worrying about breaking the binary communication, and vice versa.
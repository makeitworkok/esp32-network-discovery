#!/usr/bin/env python3
"""
ESP32 Network Discovery Tool - Code Validation Script
Validates the Arduino code structure and syntax
"""

import os
import re
import sys

def validate_file_structure():
    """Check if all required files exist"""
    required_files = [
        'NetworkDiscovery.ino',
        'config.h',
        'network_scanner.h',
        'network_scanner.cpp',
        'port_scanner.h',
        'port_scanner.cpp',
        'web_interface.h',
        'web_interface.cpp',
        'wifi_manager.h',
        'wifi_manager.cpp'
    ]
    
    missing_files = []
    for file in required_files:
        if not os.path.exists(file):
            missing_files.append(file)
    
    if missing_files:
        print(f"❌ Missing files: {', '.join(missing_files)}")
        return False
    else:
        print("✅ All required files present")
        return True

def validate_includes():
    """Check if all includes are properly defined"""
    # Pattern to match both <> and "" includes
    include_pattern = r'#include\s+[<\"]([^<>\"]+)[<>\"]'
    
    with open('NetworkDiscovery.ino', 'r') as f:
        content = f.read()
    
    includes = re.findall(include_pattern, content)
    print(f"✅ Found {len(includes)} includes: {', '.join(includes)}")
    
    # Check for required ESP32 libraries
    required_libs = ['ETH.h', 'WiFi.h']
    for lib in required_libs:
        if lib not in includes:
            print(f"❌ Missing required library: {lib}")
            return False
    
    # Check for local includes
    local_includes = ['config.h', 'network_scanner.h', 'port_scanner.h', 'web_interface.h', 'wifi_manager.h']
    for inc in local_includes:
        if inc not in includes:
            print(f"❌ Missing local include: {inc}")
            return False
    
    return True

def validate_syntax():
    """Basic syntax validation"""
    files_to_check = [
        'NetworkDiscovery.ino',
        'network_scanner.cpp',
        'port_scanner.cpp',
        'web_interface.cpp',
        'wifi_manager.cpp'
    ]
    
    for file in files_to_check:
        with open(file, 'r') as f:
            content = f.read()
        
        # Check for basic syntax issues
        open_braces = content.count('{')
        close_braces = content.count('}')
        
        if open_braces != close_braces:
            print(f"❌ Brace mismatch in {file}: {open_braces} open, {close_braces} close")
            return False
    
    print("✅ Basic syntax validation passed")
    return True

def validate_configuration():
    """Check configuration file"""
    with open('config.h', 'r') as f:
        content = f.read()
    
    # Check for required definitions
    required_defines = [
        'ETH_CONNECTION_TIMEOUT',
        'SCAN_TIMEOUT',
        'PORT_TIMEOUT',
        'TARGET_PORTS'
    ]
    
    for define in required_defines:
        if define not in content:
            print(f"❌ Missing configuration: {define}")
            return False
    
    print("✅ Configuration validation passed")
    return True

def print_compilation_instructions():
    """Print instructions for compiling the code"""
    print("\n" + "="*60)
    print("COMPILATION INSTRUCTIONS")
    print("="*60)
    print("""
To compile and upload this ESP32 network discovery tool:

1. Arduino IDE Setup:
   - Install Arduino IDE 2.0 or later
   - Add ESP32 board support in Board Manager
   - Install ESP32 board package

2. Hardware Setup:
   - Connect ESP32 with ethernet capability
   - Connect ethernet cable to network
   - Connect USB cable for programming

3. Upload Process:
   - Open NetworkDiscovery.ino in Arduino IDE
   - Select Board: ESP32 Dev Module (or your specific board)
   - Select correct COM port
   - Click Upload button

4. Monitoring:
   - Open Serial Monitor (115200 baud)
   - Watch network discovery results
   - Use serial commands (scan, status, help)

5. Target Ports Tested:
   - Port 80: HTTP
   - Port 443: HTTPS  
   - Port 502: MODBUS TCP
   - Port 47808: BACnet

6. Web Interface Features:
   - Configurable network settings (DHCP/Static IP)
   - Customizable scan ranges and target ports
   - Real-time scan progress monitoring
   - CSV export of scan results
   - Responsive web interface accessible via browser

7. Required Libraries:
   - ESP32 Arduino Core
   - ArduinoJson (install via Library Manager)
   - WebServer (included with ESP32 core)
   - SPIFFS (included with ESP32 core)

The tool provides both serial interface and web-based control
for comprehensive network discovery and management.
""")

def main():
    print("ESP32 Network Discovery Tool - Code Validation")
    print("=" * 50)
    
    validation_passed = True
    
    validation_passed &= validate_file_structure()
    validation_passed &= validate_includes()
    validation_passed &= validate_syntax()
    validation_passed &= validate_configuration()
    
    if validation_passed:
        print("\n✅ All validations passed!")
        print("Your ESP32 network discovery tool is ready for compilation.")
    else:
        print("\n❌ Some validations failed.")
        print("Please fix the issues before uploading to ESP32.")
    
    print_compilation_instructions()
    
    return 0 if validation_passed else 1

if __name__ == "__main__":
    sys.exit(main())
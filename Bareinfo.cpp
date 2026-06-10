#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <iomanip>


class CPUInfo{
public:
    std::string CPUName() {return ReadF("/proc/cpuinfo", "model name");}
    std::string CPUCores() {return ReadF("/proc/cpuinfo", "cpu cores");}
    std::string CPUThreads() {return ReadF("/proc/cpuinfo", "siblings");}
    std::string CPUMicrocode() {return ReadF("/proc/cpuinfo", "microcode");}

private:
    std::string ReadF(const std::string& fileName,
                    const std::string& key) {
        std::ifstream file(fileName);
        std::string line;

        while (std::getline(file, line)) {
            if (line.find(key) != std::string::npos) {
                size_t colon = line.find(':');

                if (colon != std::string::npos) {
                    std::string value = line.substr(colon + 1);
                    // Remove leading spaces
                    value.erase(0, value.find_first_not_of(" \t"));


                    return value;
                }
            }
        }

        return "N/A";
    }

};

class HardwareInfo {
public:
    // BIOS info
    std::string BIOSDate() {return Read("/sys/class/dmi/id/bios_date");}
    std::string BIOSVendor() {return Read("/sys/class/dmi/id/bios_vendor");}
    std::string BIOSVersion() {return Read("/sys/class/dmi/id/bios_version");}
    std::string BIOSRelease() {return Read("/sys/class/dmi/id/bios_version");}
    // Mamaboard info
    std::string MotherboardVendor() {return Read("/sys/class/dmi/id/board_vendor");}
    std::string MotherboardVersion() {return Read("/sys/class/dmi/id/board_version");}
    std::string MotherboardName() {return Read("/sys/class/dmi/id/board_name");}
    // Product info
    std::string ProductName() {return  Read("/sys/class/dmi/id/product_name");}
    std::string ProductFamily() {return Read("/sys/class/dmi/id/product_family");}
    std::string ProductVersion() {return Read("/sys/class/dmi/id/product_version");}
    // Chassis info
    std::string ChassisVendor() {return Read("/sys/class/dmi/id/chassis_vendor");}
    std::string ChassisVersion() {return Read("/sys/class/dmi/id/chassis_version");}


private:
    std::string Read(const std::string &file) {
            if (std::filesystem::exists(file)){
            std::ifstream f(file);
            std::string text;
            std::getline(f, text);
            return text;
        }
        return "N/A";
    }

};

double RAM() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    double totalRAM = 0.0;

    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            size_t start = line.find_first_of("0123456789");
            size_t end = line.find(" kB", start);
            if (start != std::string::npos && end != std::string::npos) {
                totalRAM = std::stod(line.substr(start, end - start)) / 1024.0; // Convert to GB
            }
            break;
        }
    }

    return totalRAM;
}

std::string BootMode() {
    if (std::filesystem::exists("/sys/firmware/efi")) {
        return "UEFI";
    } else {
        return "Legacy BIOS";
    }
}

bool isSecureBootEnabled() {
    std::ifstream secureBootFile("/sys/firmware/efi/vars/SecureBoot-*/data");
    if (secureBootFile.is_open()) {
        char value;
        secureBootFile.read(&value, 1);
        return value == 1; // Secure Boot is enabled if the value is 1
    }
    return false; // If the file cannot be read, Secure Boot is probably not enabled
}
double UsedRAM () {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    double totalRAM = 0.0;
    double freeRAM = 0.0;

    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            size_t start = line.find_first_of("0123456789");
            size_t end = line.find(" kB", start);
            if (start != std::string::npos && end != std::string::npos) {
                totalRAM = std::stod(line.substr(start, end - start)) / 1024.0; // Convert to GB
            }
        } else if (line.find("MemFree:") == 0) {
            size_t start = line.find_first_of("0123456789");
            size_t end = line.find(" kB", start);
            if (start != std::string::npos && end != std::string::npos) {
                freeRAM = std::stod(line.substr(start, end - start)) / 1024.0; // Convert to GB
            }
        }
    }

    return totalRAM - freeRAM; 
}

std::string KernelInfo() {
    std::ifstream versionFile("/proc/version");
    std::string kernelInfo;

    if (versionFile.is_open()) {
        std::getline(versionFile, kernelInfo);
        return kernelInfo;
    }

    return "N/A";
}

std::string DistroInfo() {
    std::ifstream osRelease("/etc/os-release");
    std::string line;
    std::string distroName = "Unknown";

    while (std::getline(osRelease, line)) {
        if (line.find("PRETTY_NAME=") == 0) {
            size_t start = line.find('"') + 1;
            size_t end = line.find('"', start);
            if (start != std::string::npos && end != std::string::npos) {
                distroName = line.substr(start, end - start);
            }
            break;
        }
    }

    return distroName;
}

std::string BatteryInfo() {
    std::ifstream batteryFile("/sys/class/power_supply/BAT0/capacity");
    std::string batteryInfo;

    if (batteryFile.is_open()) {
        std::getline(batteryFile, batteryInfo);
        return batteryInfo + "%";
    }

    return "N/A";
}

std::string DiskInfo() {
    auto human_size = [](unsigned long long bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double size = static_cast<double>(bytes);
        while (size >= 1024.0 && unit < 4) {
            size /= 1024.0;
            ++unit;
        }
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(size >= 10.0 ? 0 : 2) << size << " " << units[unit];
        return ss.str();
    };

    std::string info;
    const std::filesystem::path sys_block("/sys/block");
    if (!std::filesystem::exists(sys_block)) return "N/A";

    for (auto const& entry : std::filesystem::directory_iterator(sys_block)) {
        std::string dev = entry.path().filename().string();

        if (dev.rfind("loop", 0) == 0) continue;
        if (dev.rfind("ram", 0) == 0) continue;
        if (dev == "fd0") continue;

        std::uint64_t sectors = 0;
        std::filesystem::path sizePath = entry.path() / "size";
        if (std::filesystem::exists(sizePath)) {
            std::ifstream sf(sizePath);
            std::string s;
            if (std::getline(sf, s)) {
                try {
                    sectors = std::stoull(s);
                } catch (...) { sectors = 0; }
            }
        }

        unsigned long long bytes = sectors * 512ULL;

        
        std::string model;
        std::filesystem::path modelPath = entry.path() / "device" / "model";
        std::filesystem::path vendorPath = entry.path() / "device" / "vendor";
        if (std::filesystem::exists(modelPath)) {
            std::ifstream mf(modelPath);
            std::getline(mf, model);
        } else if (std::filesystem::exists(vendorPath)) {
            std::ifstream vf(vendorPath);
            std::getline(vf, model);
        } else {
            std::filesystem::path alt = entry.path() / "device";
            if (std::filesystem::exists(alt)) {
                for (auto const& sub : std::filesystem::directory_iterator(alt)) {
                    std::filesystem::path p = sub.path() / "model";
                    if (std::filesystem::exists(p)) {
                        std::ifstream sf2(p);
                        std::getline(sf2, model);
                        break;
                    }
                }
            }
        }

        std::ostringstream line;
        line << dev << "\t" << (bytes ? human_size(bytes) : "Unknown size");
        if (!model.empty()) line << "\t" << model;
        info += line.str() + "\n";
    }

    return info.empty() ? "N/A" : info;
}

std::string ShellInfo() {
    const char* shell = std::getenv("SHELL");
    return shell ? shell : "N/A";
}

int main () {
    // Colors so the output doesn't look boring
    const std::string RESET  = "\033[0m";
    const std::string RED    = "\033[1;31m";
    const std::string GREEN  = "\033[1;32m";
    const std::string YELLOW = "\033[1;33m";
    const std::string BLUE   = "\033[1;34m";
    const std::string PURPLE = "\033[1;35m";
    const std::string CYAN   = "\033[1;36m";
    const std::string PINK   = "\033[1;95m";
    const std::string WHITE  = "\033[1;37m";
    
    CPUInfo cpuinfo;
    HardwareInfo hwinfo;
    std::cout << BLUE << "CPU Name:             " << cpuinfo.CPUName() << RESET << "\n";
    std::cout << BLUE << "CPU Cores:            " << cpuinfo.CPUCores() << RESET << "\n";
    std::cout << BLUE << "CPU Threads:          " << cpuinfo.CPUThreads() << RESET << "\n";
    std::cout << BLUE << "CPU Microcode:        " << cpuinfo.CPUMicrocode() << RESET << "\n";
    std::cout << PURPLE << "BIOS Vendor:        " << hwinfo.BIOSVendor() << RESET << "\n";
    std::cout << PURPLE << "BIOS Release:       " << hwinfo.BIOSRelease() << RESET << "\n";
    std::cout << PURPLE << "BIOS Date:          " << hwinfo.BIOSDate() << RESET << "\n";
    std::cout << PURPLE << "BIOS Version:       " << hwinfo.BIOSVersion() << RESET << "\n"; 
    std::cout << GREEN << "Motherboard Vendor:  " << hwinfo.MotherboardVendor() << RESET << "\n";
    std::cout << GREEN << "Motherboard Version: " << hwinfo.MotherboardVersion() << RESET << "\n";
    std::cout << GREEN << "Motherboard Name:    " << hwinfo.MotherboardName() << RESET << "\n";
    std::cout << YELLOW << "Product Name:       " << hwinfo.ProductName() << RESET << "\n";
    std::cout << YELLOW << "Product Family:     " <<  hwinfo.ProductFamily() << RESET << "\n";
    std::cout << YELLOW << "Product Version:    " << hwinfo.ProductVersion() << RESET << "\n";
    std::cout << CYAN << "Chassis Vendor:     " << hwinfo.ChassisVendor() << RESET << "\n";
    std::cout << CYAN << "Chassis Version:    " << hwinfo.ChassisVersion() << RESET << "\n";
    std::cout << RED << "Total RAM (GB):     " << RAM() << RESET << "\n";
    std::cout << RED << "Used RAM (GB):      " << UsedRAM() << RESET << "\n";
    std::cout << RED << "Secure Boot:        " << (isSecureBootEnabled() ? "Enabled" : "Disabled") << RESET << "\n";
    std::cout << PINK << "Distro:             " << DistroInfo() << RESET << "\n";
    std::cout << PINK << "Kernel:             " << KernelInfo() << RESET << "\n";
    if (BatteryInfo() != "N/A") {
        std::cout << PINK << "Battery:            " << BatteryInfo() << RESET << "\n";
    }
    std::cout << PINK << "Shell:              " << ShellInfo() << RESET << "\n";
    std::cout << PINK << "Disk Info:          " << DiskInfo() << RESET;
    std::cout << PINK << "Boot Mode:          " << BootMode() << RESET << "\n";
    return 0;
}

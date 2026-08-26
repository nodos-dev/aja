#pragma once

#include <map>
#include <string>

// Dates come from R5-Prerequisites, Prerequisites/AJA/firmware/versions.txt on
// branch reality-5.8. Keys are what CNTV2Card::GetModelName() returns.
const std::map<std::string, std::string> TESTED_FIRMWARES =
{
    { "Corvid44-12G-8K", "2025/04/21" },
    { "Corvid88",        "2025/10/22" },
    { "Corvid44",        "2025/10/22" },
};
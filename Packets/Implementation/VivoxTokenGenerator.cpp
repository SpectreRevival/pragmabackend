#include <VivoxTokenGenerator.h>
#include <array>
#include <ctime>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <random>
#include <sstream>
// #include "spdlog/spdlog.h"

static const std::string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static unsigned int globalVxi = 0;

std::string VivoxTokenGenerator::base64url_encode(const std::string& in) {
    std::string out;
    int val = 0;
    int valb = -6;

    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    return out;
}

std::string VivoxTokenGenerator::hmac_sha256_b64url(const std::string& key, const std::string& msg) {
    std::array<unsigned char, 32> hash{};
    unsigned int len = 32;

    HMAC(
        EVP_sha256(),
        key.data(),
        static_cast<int>(key.size()),
        reinterpret_cast<const unsigned char*>(msg.data()), // NOLINT
        msg.size(),
        hash.data(),
        &len);

    return base64url_encode(std::string(reinterpret_cast<char*>(hash.data()), len)); // NOLINT
}

std::string VivoxTokenGenerator::Generate(
    const std::string& secretKey,
    const std::string& issuer,
    const std::string& domain,
    const std::string& subject,
    const std::string& action,
    const std::string& channel) {
    const int exp = static_cast<int>(std::time(nullptr) + 100);
    const int vxi = ++globalVxi;

    const std::string from =
        "sip:." + issuer + "." + subject + ".@" + domain;

    std::stringstream payload;

    payload << "{";
    payload << R"("iss":")" << issuer << "\",";
    payload << "\"exp\":" << exp << ",";
    payload << R"("vxa":")" << action << "\",";
    payload << "\"vxi\":" << vxi << ",";
    payload << R"("f":")" << from << "\"";

    if (action == "join") {
        payload << R"(,"t":"sip:confctl-g-)"
                << issuer << "." << channel << "@"
                << domain << "\"";
    }

    payload << "}";

    const std::string header = "e30"; // empty header \ {}
    const std::string payloadB64 = base64url_encode(payload.str());

    const std::string signingInput = header + "." + payloadB64;
    const std::string signature = hmac_sha256_b64url(secretKey, signingInput);

    // spdlog::info("Generated Token {}", signing_input + "." + signature);

    return signingInput + "." + signature;
}
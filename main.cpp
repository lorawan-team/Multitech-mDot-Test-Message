#include "mbed.h"
#include "mDot.h"
#include "MTSLog.h"
#include <string>
#include <vector>
#include <algorithm>

// these options must match the settings on your Conduit
// uncomment the following lines and edit their values to match your configuration
// You can get these credentials from someone at Delta ;)
static std::string config_network_name = "____DELTA NETWORK CREDENTIALS____";
static std::string config_network_pass = "____DELTA NETWORK CREDENTIALS____";
static uint8_t config_frequency_sub_band = 1;

int main() {
    int32_t ret;
    mDot* dot;
    std::vector<uint8_t> data;
    std::string data_str = "{\"Bridge\": false}";
    
    // get a mDot handle
    dot = mDot::getInstance();
    
    // print library version information
    logInfo("version: %s", dot->getId().c_str());

    //*******************************************
    // configuration
    //*******************************************
    // reset to default config so we know what state we're in
    dot->resetConfig();
    
    dot->setLogLevel(mts::MTSLog::INFO_LEVEL);

    // set up the mDot with our network information: frequency sub band, network name, and network password
    // these can all be saved in NVM so they don't need to be set every time - see mDot::saveConfig()
    
    // frequency sub band is only applicable in the 915 (US) frequency band
    // if using a MultiTech Conduit gateway, use the same sub band as your Conduit (1-8) - the mDot will use the 8 channels in that sub band
    // if using a gateway that supports all 64 channels, use sub band 0 - the mDot will use all 64 channels
    logInfo("setting frequency sub band");
    if ((ret = dot->setFrequencySubBand(config_frequency_sub_band)) != mDot::MDOT_OK) {
        logError("failed to set frequency sub band %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
    }
    
    logInfo("setting network name");
    if ((ret = dot->setNetworkName(config_network_name)) != mDot::MDOT_OK) {
        logError("failed to set network name %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
    }
    
    logInfo("setting network password");
    if ((ret = dot->setNetworkPassphrase(config_network_pass)) != mDot::MDOT_OK) {
        logError("failed to set network password %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
    }
    
    // a higher spreading factor allows for longer range but lower throughput
    // in the 915 (US) frequency band, spreading factors 7 - 10 are available
    // in the 868 (EU) frequency band, spreading factors 7 - 12 are available
    logInfo("setting TX spreading factor");
    if ((ret = dot->setTxDataRate(mDot::SF_10)) != mDot::MDOT_OK) {
        logError("failed to set TX datarate %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
    }
    
    // request receive confirmation of packets from the gateway
    logInfo("enabling ACKs");
    if ((ret = dot->setAck(1)) != mDot::MDOT_OK) {
        logError("failed to enable ACKs %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
    }
    
    // set join mode to AUTO_OTA so the mDot doesn't have to rejoin after sleeping
    logInfo("setting join mode to AUTO_OTA");
    if ((ret = dot->setJoinMode(mDot::AUTO_OTA)) != mDot::MDOT_OK) {
        logError("failed to set join mode %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
    }
    
    // save this configuration to the mDot's NVM
    logInfo("saving config");
    if (! dot->saveConfig()) {
        logError("failed to save configuration");
    }
    //*******************************************
    // end of configuration
    //*******************************************

    // format data for sending to the gateway
    for (std::string::iterator it = data_str.begin(); it != data_str.end(); it++)
        data.push_back((uint8_t) *it);

    // join the network if not joined
    if (!dot->getNetworkJoinStatus()) {
        logInfo("network not joined, joining network");
        if ((ret = dot->joinNetwork()) != mDot::MDOT_OK) {
            logError("failed to join network %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
        }
    }
    if (dot->getNetworkJoinStatus()) {
        // send the data
        // ACKs are enabled by default, so we're expecting to get one back
        if ((ret = dot->send(data)) != mDot::MDOT_OK) {
            logError("failed to send %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
        } else {
            logInfo("successfully sent data to gateway");
        }
    }

    // in the 868 (EU) frequency band, we need to wait until another channel is available before transmitting again
    uint32_t sleep_time = std::max((uint32_t)10000, (uint32_t)dot->getNextTxMs()) / 1000;
    logInfo("going to sleep...");

    // go to deepsleep and wake up automatically sleep_time seconds later
    dot->sleep(sleep_time, mDot::RTC_ALARM);

    // go to deepsleep and wake up on rising edge of WKUP pin (PA0/XBEE_CTS/XBEE_DIO7)
    // dot->sleep(0, mDot::INTERRUPT);

    return 0;
}

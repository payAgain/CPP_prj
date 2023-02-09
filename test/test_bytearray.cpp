#include "d_bytearray.h"
#include "log.h"
#include <arpa/inet.h>

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

int main() {
    dreamer::ByteArray bt(5);
    bt.writeFixInt(1000);
    bt.writeStringWithFixLength("Hello World");
    bt.writeUint64(19241);
    bt.writeFloat(3.14);
    bt.writeStringWithUnfixLength("nihao shijie!");
    // bt.resetPos(0);
    // bt.writeToFile("/home/parallels/Desktop/output.txt", -1);
    
    
    // bt.resetPos(0);
    // bt.readFromFile("/home/parallels/Desktop/output.txt", -1);
    bt.resetPos(0);
    int res;
    bt.readFixInt(res);
    D_SLOG_INFO(g_logger) << res;
    D_SLOG_INFO(g_logger) << bt.readFixLengthString();
    D_SLOG_INFO(g_logger) << bt.readUint64();
    D_SLOG_INFO(g_logger) << bt.readFloat();
    D_SLOG_INFO(g_logger) << bt.readUnifxLengthString();
    // auto g =  bt.toString();
    // "\000\000\003\350\000\000\000\000\000\000\000\vHello World\251\226\001"
    // "\000\000\003\350\000\000\000\000\000\000\000\000Hello WoHel\251\226\001"
    // "\000\000\003\350\000\000\000\000\000\000\000\000Hello WoHel\251\226\001"
}
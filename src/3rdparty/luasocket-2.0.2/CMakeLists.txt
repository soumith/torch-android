CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

FIND_PACKAGE(Torch REQUIRED)
SET(src auxiliar.c except.c io.c mime.c select.c timeout.c unix.c buffer.c inet.c luasocket.c options.c tcp.c udp.c usocket.c)
SET(luasrc socket.lua http.lua ftp.lua ltn12.lua mime.lua smtp.lua tp.lua url.lua)

ADD_TORCH_PACKAGE(socket "${src}" "${luasrc}" "LuaSocket")
# TARGET_LINK_LIBRARIES(socket lua)

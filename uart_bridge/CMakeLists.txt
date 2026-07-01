cmake_minimum_required(VERSION 3.8)

project(uart_bridge C)

add_executable(uart_bridge main.c)

install(TARGETS uart_bridge RUNTIME DESTINATION bin)
install(PROGRAMS S20uart_bridge DESTINATION /etc/init.d)

/* Copyright (c) 2019 ARM, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ntp-client/NTPClient.h"
#include "mbed.h"

NTPClient::NTPClient(NetworkInterface *interface)
    : iface(interface), nist_server_address(NTP_DEFULT_NIST_SERVER_ADDRESS), nist_server_port(NTP_DEFULT_NIST_SERVER_PORT) {
}

void NTPClient::set_server(const char* server, int port) {
    nist_server_address = server;
    nist_server_port = port;
}

int NTPClient::get_timestamp(time_t &timestamp, int timeout) {
    const time_t TIME1970 = (time_t)2208988800UL;
    int ntp_send_values[12] = {0};
    int ntp_recv_values[12] = {0};

    SocketAddress nist;

    if (iface) {
        int ret_gethostbyname = iface->gethostbyname(nist_server_address, &nist);

        if (ret_gethostbyname < 0) {
            // Network error on DNS lookup
            return ret_gethostbyname;
        }

        nist.set_port(nist_server_port);

        memset(ntp_send_values, 0x00, sizeof(ntp_send_values));
        ntp_send_values[0] = '\x1b';

        memset(ntp_recv_values, 0x00, sizeof(ntp_recv_values));

        UDPSocket sock;
        sock.open(iface);
        sock.set_timeout(timeout);

        sock.sendto(nist, (void*)ntp_send_values, sizeof(ntp_send_values));

        SocketAddress source;
        const int n = sock.recvfrom(&source, (void*)ntp_recv_values, sizeof(ntp_recv_values));

        if (n > 10) {
            timestamp = ntohl(ntp_recv_values[10]) - TIME1970;
            return 0;
        } else {
            if (n < 0) {
                // Network error
                return n;

            } else {
                // No or partial data returned
                return -1;
            }
        }

    } else {
        // No network interface
        return -2;
    }
}

time_t NTPClient::get_timestamp(int timeout) {
    time_t timestamp;
    int ret = get_timestamp(timestamp, timeout);
    if (ret < 0) {
        // This doesn't work with the ARM toolchain whose time_t is unsigned
        // Please use the new API get_timestamp(time_t *timestamp, int timeout)
        return ret;
    } else {
        return timestamp;
    }
}

void NTPClient::network(NetworkInterface *interface) {
    iface = interface;
}

uint32_t NTPClient::ntohl(uint32_t x) {
    uint32_t ret = (x & 0xff) << 24;
    ret |= (x & 0xff00) << 8;
    ret |= (x & 0xff0000UL) >> 8;
    ret |= (x & 0xff000000UL) >> 24;
    return ret;
}

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip-debug.h"

#include <string.h>
#include <stdlib.h>

#define DEBUG DEBUG_PRINT
#define UDP_IP_BUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);

static void tcpip_handler() {
        int val;
        char buf[MAX_PAYLOAD_LEN];

        if(uip_newdata()) { 
                ((char *)uip_appdata)[uip_datalen()] = 0;
                PRINTF("Server received: '%s' from ", (char *)uip_appdata);
        PRINT6ADDR(&UDP_IP_BUF->srcipaddr);
        PRINTF("\n");
  
        val = atoi(uip_appdata);
        val *= 2;

        uip_ipaddr_copy(&server_conn->ripaddr, &UDP_IP_BUF->srcipaddr);
        server_conn->rport = UDP_IP_BUF->srcport;
        PRINTF("Responding with message: ");
        sprintf(buf, "%d", val);
        PRINTF("%s\n", buf);
  
        uip_udp_packet_send(server_conn, buf, strlen(buf));

        memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
        server_conn->rport = 0;
  }
}

static void print_local_addresses() {
        int i;
        uint8_t state;

        PRINTF("Server IPv6 addresses: ");
        for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
                state = uip_ds6_if.addr_list[i].state;
                if(uip_ds6_if.addr_list[i].isused && 
                        (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
                PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
                PRINTF("\n");
    }
  }
}

PROCESS_THREAD(udp_server_process, ev, data) {
        static struct etimer timer;

        PROCESS_BEGIN();
        PRINTF("UDP server started\n");
 
        etimer_set(&timer, CLOCK_CONF_SECOND*3);
        
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        print_local_addresses();
        
        server_conn = udp_new(NULL, UIP_HTONS(4444), NULL);
        udp_bind(server_conn, UIP_HTONS(3333));
        PRINTF("Server listening on UDP port %u\n", UIP_HTONS(server_conn->lport));

        while(1) {
                PROCESS_YIELD();
                if(ev == tcpip_event) {
                        tcpip_handler();
                }
        }
        
  PROCESS_END();
}


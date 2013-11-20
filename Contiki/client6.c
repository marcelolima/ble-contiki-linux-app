#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip-debug.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#define SEND_INTERVAL 15 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN 40

static struct uip_udp_conn *client_conn;

PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);

static void tcpip_handler(void) {
        char *str;

        if(uip_newdata()) {
                str = uip_appdata;
                str[uip_datalen()] = '\0';
                printf("Response from the server: '%s'\n", str);
        }
}

static void timeout_handler(void) {
        char *val = "1";
        char buf[MAX_PAYLOAD_LEN];
        printf("Client sending to: ");
        PRINT6ADDR(&client_conn->ripaddr);
        sprintf(buf, "%s", val);
        printf(" (msg: %s)\n", buf);
        uip_udp_packet_send(client_conn, buf, 
                strlen(buf));
}

static void print_local_addresses(void) {
        int i;
        uint8_t state;

        PRINTF("Server IPv6 addresses: ");
        for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
                state = uip_ds6_if.addr_list[i].state;
                if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE ||state == ADDR_PREFERRED)) {
                        PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
                        PRINTF("\n");
                }
        }
}

PROCESS_THREAD(udp_client_process, ev, data)
{
        static struct etimer et;
        uip_ipaddr_t ipaddr;
        
        PROCESS_BEGIN();
        
        PRINTF("UDP client process started\n");

        etimer_set(&et, CLOCK_CONF_SECOND*3);
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        print_local_addresses();

        //set the server ip
        uip_ip6addr(&ipaddr,0xfe80,0,0,0,0x0206,0x98ff,0xfe00,0x0232);

        client_conn = udp_new(&ipaddr, UIP_HTONS(3333), NULL);
        udp_bind(client_conn, UIP_HTONS(4444));

        PRINTF("Created a connection with the server ");
        PRINT6ADDR(&client_conn->ripaddr);
        PRINTF("local/remote port %u/%u\n",
        UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
        etimer_set(&et, SEND_INTERVAL);
 
        while(1) {

                PROCESS_YIELD();
    
                if(etimer_expired(&et)) {
                timeout_handler();
                etimer_restart(&et);
                } else if(ev == tcpip_event) {
                        tcpip_handler();
                }
	}

        PROCESS_END();
}

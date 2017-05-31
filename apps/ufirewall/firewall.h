/*
 * Userlevel firewall API
 */
#include<stdbool.h>
#include<stdint.h>

int firewall_init(void);
bool firewall_check_connect(uint16_t port, uint32_t ip);
bool firewall_check_bind(uint16_t port);


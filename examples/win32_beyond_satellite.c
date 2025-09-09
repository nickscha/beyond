#include "../beyond.h"
#include "../win32_beyond_satellite.h"

typedef struct satellite_packet
{
    long tick;
    float x;
    float y;

} satellite_packet;

BEYOND_API BEYOND_INLINE beyond_bool satellite_transmit(beyond_handle handle, beyond_context context, void *payload, unsigned int payload_capacity, unsigned int *payload_size)
{
    satellite_packet *packet = (satellite_packet *)context;

    if (payload_capacity < (int)sizeof(satellite_packet))
    {
        return false;
    }

    beyond_satellite_lock(handle);
    packet->y += 0.1f;
    beyond_satellite_unlock(handle);

    *payload_size = sizeof(satellite_packet);

    win32_printf("[satellite] data: %.3lf, %.3lf", (double)packet->x, (double)packet->y);

    beyond_memcpy(((char *)payload), packet, sizeof(satellite_packet));

    return true;
}

BEYOND_API BEYOND_INLINE beyond_bool satellite_receive(beyond_handle handle, beyond_context context, void *payload, unsigned int payload_capacity, unsigned int payload_size)
{
    win32_printf("recieved: %d", payload_size);

    (void)handle;
    (void)context;
    (void)payload;
    (void)payload_capacity;
    (void)payload_size;

    return true;
}

#define TRANSMIT_PAYLOAD_CAPACITY 1024
#define RECEIVE_PAYLOAD_CAPACITY 4096

#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
#ifdef __i686__
__attribute((force_align_arg_pointer))
#endif
int
mainCRTStartup(void)
{
    char transmit_payload[TRANSMIT_PAYLOAD_CAPACITY];
    char receive_payload[RECEIVE_PAYLOAD_CAPACITY];

    satellite_packet context = {0};

    beyond_handle satellite;
    beyond_satellite_api api = {0};

    api.type = BEYOND_COMMUNICATION_TYPE_UDP;
    api.transmit_rate_hz = 60;
    api.transmit = satellite_transmit;
    api.receive = satellite_receive;
    api.address = beyond_address_init("127.0.0.1", 8080);
    api.context = &context;
    api.transmit_payload = transmit_payload;
    api.transmit_payload_capacity = TRANSMIT_PAYLOAD_CAPACITY;
    api.receive_payload = receive_payload;
    api.receive_payload_capacity = RECEIVE_PAYLOAD_CAPACITY;

    satellite = beyond_satellite_deploy(&api);

    if (!satellite)
    {
        return 1;
    }

    beyond_satellite_lock(satellite);
    context.x = 600.0f;
    context.y = 10.0f;
    beyond_satellite_unlock(satellite);

    /* Run the main application loop */
    while (1)
    {
        beyond_satellite_lock(satellite);
        context.x += 1.0f;
        beyond_satellite_unlock(satellite);
        Sleep(100);
    }

    beyond_satellite_destroy(satellite);

    return 0;
}

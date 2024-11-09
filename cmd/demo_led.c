#include <common.h>
#include <command.h>
#include <dm.h>
#include <led.h>
#include <dm/uclass-internal.h>

static const char *const state_label[] = {
    [LEDST_OFF]    = "off",
    [LEDST_ON]    = "on",
    [LEDST_TOGGLE]    = "toggle",
};

enum led_state_t get_demo_led_cmd(char *var)
{
    int i;

    for (i = 0; i < LEDST_COUNT; i++) {
        if (!strncmp(var, state_label[i], strlen(var)))
            return i;
    }

    return -1;
}

static int show_led_state(struct udevice *dev)
{
    int ret;

    ret = led_get_state(dev);
    if (ret >= LEDST_COUNT)
        ret = -EINVAL;
    if (ret >= 0)
        printf("%s\n", state_label[ret]);

    return ret;
}

static int show_label(void)
{
    struct udevice *dev;
    for (uclass_find_first_device(UCLASS_LED, &dev);
         dev;
         uclass_find_next_device(&dev)) {
        struct led_uc_plat *plat = dev_get_uclass_plat(dev);

        if (!plat->label)
            continue;
        printf("%s \n", plat->label);
    }

    return 0;
}

int do_demo_led(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
    enum led_state_t cmd;
    const char *led_label;
    struct udevice *dev;

    int ret;

    /* Validate arguments */
    if (argc < 2)
        return CMD_RET_USAGE;
    led_label = argv[1];
    if (strncmp(led_label, "show", 4) == 0)
        return show_label();

    cmd = argc > 2 ? get_demo_led_cmd(argv[2]) : LEDST_COUNT;

    ret = led_get_by_label(led_label, &dev);
    if (ret) {
        printf("LED '%s' not found (err=%d)\n", led_label, ret);
        return CMD_RET_FAILURE;
    }
    switch (cmd) {
    case LEDST_OFF:
    case LEDST_ON:
    case LEDST_TOGGLE:
        ret = led_set_state(dev, cmd);
        break;

    case LEDST_COUNT:
        printf("LED '%s': ", led_label);
        ret = show_led_state(dev);
        break;
    }
    if (ret < 0) {
        printf("LED '%s' operation failed (err=%d)\n", led_label, ret);
        return CMD_RET_FAILURE;
    }

    return 0;
}

U_BOOT_CMD(
    demo_led, 4, 1, do_demo_led,
    "manage DEMO LEDs",
    "<led_label> on|off|toggle\tChange LED state\n"
    "demo_led <led_label>\tGet LED state\n"
    "demo_led show\t Show LED label"
);


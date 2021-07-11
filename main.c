#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include "ata_smart.h"

typedef struct apm_info {
        int state;
        int value;
} apm_info_t;

enum {
        APM_READ_ONLY = 0,
        APM_SET_ENABLE,
        APM_SET_DISABLE,
        NUM_APM_SET_ACTIONS,
};

static int drive_phyid = -1;
static int target_id = 0xA0; // 0xA0 0xB0
static int action = APM_READ_ONLY;
static int new_apm_value = 0;

void help(void)
{
        printf("Usage:\n");
        printf("    apm_set.exe [options]\n");
        printf("\n");
        printf("Options:\n");
        printf("    -i <...>    physical drive index: \\\\.\\PhysicalDrive[X]\n");
        printf("    -r          read apm state and value only (default)\n");
        printf("    -e <...>    enable apm with value: 1 (minimal power) - 254 (max performance)\n");
        printf("    -d          disable apm\n");
        printf("    -h          print this help\n");
}

int parse_opts(int argc, char *argv[])
{
        int v;
        int c;

        opterr = 0;

        // Handle single character options (-X)
        while ((c = getopt(argc, argv, "i:re:dh")) != -1) {
                switch (c) {
                case 'h':
                        help();
                        exit(-EINVAL);

                case 'i':
                        if (sscanf(optarg, "%d", &v) != 1)
                                return -EINVAL;

                        if (v < 0) {
                                fprintf(stderr, "invalid physical disk index: %d\n", v);
                                return -EINVAL;
                        }

                        drive_phyid = v;

                        break;

                case 'r':
                        action = APM_READ_ONLY;
                        break;

                case 'd':
                        action = APM_SET_DISABLE;
                        break;

                case 'e':
                        if (sscanf(optarg, "%d", &v) != 1)
                                return -EINVAL;

                        // 0 is NOT allowed
                        if (v < 1 || v >= 255) {
                                fprintf(stderr, "invalid apm value: %d\n", v);
                                return -EINVAL;
                        }

                        action = APM_SET_ENABLE;
                        new_apm_value = v;

                        break;

                case '?':
                        if (optopt == 'i' || optopt == 'e') // Case for missing option argument(s)
                                fprintf(stderr, "option -%c needs argument\n", optopt);
                        else if (isprint(optopt)) // Case for undefined options
                                fprintf(stderr, "unknown option -%c\n", optopt);
                        else // Case for not ANSI chars options
                                fprintf(stderr, "failed to parse char: \\x%x\n", optopt);

                        return -EINVAL;

                default:
                        break;
                }
        }

        // Handle non-option arguments
        for (int i = optind; i < argc; i++) {
                fprintf(stdout, "Non-option args: %s\n", argv[i]);
        }

        return 0;
}

void apm_info_get(IDENTIFY_DEVICE *ident, apm_info_t *res)
{
        res->state = (ident->A.CommandSetEnabled2 & (1 << 3)) ? 1 : 0;
        res->value = GetApmValue(ident);
}

int main(int argc, char *argv[])
{
        IDENTIFY_DEVICE ident = { 0 };
        apm_info_t apm_info = { 0 };
        int err = 0;

        if ((err = parse_opts(argc, argv)))
                goto out;

        if (drive_phyid < 0) {
                fprintf(stderr, "physical drive index is not specified\n");
                err = -EINVAL;
                goto out;
        }

        AtaSmartInit();

        if (WakeUp(drive_phyid) == FALSE) {
                fprintf(stderr, "failed to wake up disk %d\n", drive_phyid);
                err = -EIO;
                goto out;
        }

        target_id = 0xA0;
        if (DoIdentifyDevicePd(drive_phyid, target_id, &ident) == FALSE) {
                WakeUp(drive_phyid);

                target_id = 0xB0;
                if (DoIdentifyDevicePd(drive_phyid, target_id, &ident) == FALSE) {
                        fprintf(stderr, "failed to read info, not supported disk\n");
                        err = -ENOTSUP;
                        goto out;
                }
        }

        if ((ident.A.CommandSetSupported2 & (1 << 3)) == 0) {
                fprintf(stderr, "this disk \"%.*s\" does not support APM feature\n",
                        (int)sizeof(ident.A.Model), ident.A.Model);
                err = -ENOTSUP;
                goto out;
        }

        switch (action) {
        case APM_READ_ONLY:
                goto print_res;

        case APM_SET_ENABLE:
                EnableApm(drive_phyid, target_id, new_apm_value);
                break;

        case APM_SET_DISABLE:
                DisableApm(drive_phyid, target_id);
                break;

        default:
                fprintf(stderr, "invalid action\n");
                goto out;
        }

        usleep(10 * 1000);

        if (DoIdentifyDevicePd(drive_phyid, target_id, &ident) == FALSE) {
                fprintf(stderr, "failed to read disk info\n");
                err = -EIO;
                goto out;
        }

print_res:
        apm_info_get(&ident, &apm_info);

        printf("current apm state: %u\n", apm_info.state);
        printf("current apm value: %u\n", apm_info.value);

out:
        return err;
}
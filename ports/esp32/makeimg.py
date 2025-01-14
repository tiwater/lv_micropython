# Combine bootloader, partition table, nvs, and application into a final binary.

import os, sys

sys.path.append(os.getenv("IDF_PATH") + "/components/partition_table")

import gen_esp32part

OFFSET_BOOTLOADER_DEFAULT = 0x1000
OFFSET_PARTITIONS_DEFAULT = 0x8000
MAX_SIZE_PARTITIONS_DEFAULT = 0x1000
OFFSET_APP = 0x10000

def load_sdkconfig_hex_value(filename, value, default):
    value = "CONFIG_" + value + "="
    with open(filename, "r") as f:
        for line in f:
            if line.startswith(value):
                return int(line.split("=", 1)[1], 16)
    return default


def load_partition_table(filename):
    with open(filename, "rb") as f:
        return gen_esp32part.PartitionTable.from_binary(f.read())


# Extract command-line arguments.
arg_sdkconfig = sys.argv[1]
arg_bootloader_bin = sys.argv[2]
arg_partitions_bin = sys.argv[3]
arg_nvs_bin = sys.argv[4]
arg_nvs_key_bin = sys.argv[5]
arg_app_bin = sys.argv[6]
arg_output_bin = sys.argv[7]

# Load required sdkconfig values.
offset_bootloader = load_sdkconfig_hex_value(
    arg_sdkconfig, "BOOTLOADER_OFFSET_IN_FLASH", OFFSET_BOOTLOADER_DEFAULT
)
offset_partitions = load_sdkconfig_hex_value(
    arg_sdkconfig, "PARTITION_TABLE_OFFSET", OFFSET_PARTITIONS_DEFAULT
)

# Load the partition table.
partition_table = load_partition_table(arg_partitions_bin)

max_size_bootloader = offset_partitions - offset_bootloader
max_size_partitions = MAX_SIZE_PARTITIONS_DEFAULT

# Define the input files, their location and maximum size.
files_in = [
    ("bootloader", offset_bootloader, max_size_bootloader, arg_bootloader_bin),
    ("partitions", offset_partitions, max_size_partitions, arg_partitions_bin),
]
# Inspect the partition table to find offsets and maximum sizes.
for part in partition_table:
    if part.name == "nvs":
        files_in.append(("nvs", part.offset, part.size, arg_nvs_bin))
    elif part.name == "nvs_key":
        files_in.append(("nvs_key", part.offset, part.size, arg_nvs_key_bin))
    elif part.type == gen_esp32part.APP_TYPE and part.offset == OFFSET_APP:
        files_in.append(("app", part.offset, part.size, arg_app_bin))

file_out = arg_output_bin

# Write output file with combined firmware.
cur_offset = offset_bootloader
with open(file_out, "wb") as fout:
    for name, offset, max_size, file_in in files_in:
        assert offset >= cur_offset
        fout.write(b"\xff" * (offset - cur_offset))
        cur_offset = offset
        try:
            with open(file_in, "rb") as fin:
                data = fin.read()
                fout.write(data)
                cur_offset += len(data)
                print(
                    "%-12s@0x%06x % 8d  (% 8d remaining)"
                    % (name, offset, len(data), max_size - len(data))
                )
                if len(data) > max_size:
                    print(
                        "ERROR: %s overflows allocated space of %d bytes by %d bytes"
                        % (name, max_size, len(data) - max_size)
                    )
                    sys.exit(1)
        except FileNotFoundError:
            pass
    print("%-22s% 8d" % ("total", cur_offset))

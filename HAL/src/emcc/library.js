mergeInto(LibraryManager.library, {
    read_packet: function (if_index_mask, buffer, length, src_mac, dst_mac, timeout, if_index) {
        return Asyncify.handleSleep(function (wakeUp) {
            (async () => {
                await new Promise((resolve) => setTimeout(resolve, 1000));
                let len = 10;
                for (let i = 0; i < len; i++) {
                    HEAP8[buffer + i] = i;
                }
                return len;
            })().then(wakeUp);
        });
    },
    send_packet: function (if_index, buffer, length, dst_mac) {
        let output = "";
        output += `to ${if_index}: `;
        for (let i = 0; i < 6; i++) {
            output += HEAP8[dst_mac + i].toString(16).padStart(2, "0");
            if (i < 5) {
                output += ':';
            }
        }
        output += ' content:';
        for (let i = 0; i < length; i++) {
            output += HEAP8[buffer + i].toString(16).padStart(2, "0");
        }
        console.log(output);
        return 0;
    }
});
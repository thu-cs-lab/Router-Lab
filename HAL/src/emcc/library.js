mergeInto(LibraryManager.library, {
    read_packet: function (buffer) {
        console.log(buffer);
        return Asyncify.handleSleep(function (wakeUp) {
            (async () => {
                await new Promise((resolve) => setTimeout(resolve, 1000));
                return 10;
            })().then(wakeUp);
        });
    },
    send_packet: function (if_index, buffer, length, dst_mac) {
        console.log(if_index, buffer, length, dst_mac);
        return 0;
    }
});
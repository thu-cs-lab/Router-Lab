mergeInto(LibraryManager.library, {
    read_packet: function (buffer) {
        console.log(buffer);
        return Asyncify.handleSleep(function (wakeUp) {
            new Promise((resolve) => setTimeout(resolve, 1000))
                .then(() => wakeUp(0));

        });
    }
});
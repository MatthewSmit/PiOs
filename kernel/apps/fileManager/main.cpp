void loadSDProcess() {
    uint64_t size;
    auto rawFile = loadFileInternal("/apps/internal/emmc/emmc", size);

    ProcessData processData{};
    processData.data = rawFile;
    processData.size = size;
    processData.permissions.memory = ~0ULL;
    createProcess(&processData);
    waitUntilReady(&processData);
    freeInternal(rawFile);
}

void loadTarProcess() {
    uint64_t size;
    auto rawFile = loadFileInternal("/apps/internal/emmc/emmc", size);

    ProcessData processData{};
    processData.data = rawFile;
    processData.size = size;
    processData.permissions.memory = ~0ULL;
    createProcess(&processData);
    waitUntilReady(&processData);
    freeInternal(rawFile);
}

void setEvents() {
    // TODO
}

int main(int argc, const char* argv[]) {
    loadSDProcess();
    loadTarProcess();
    setEvents();
    setState(ProcessState.Loaded);

    while (true) {
        waitForEvent();
        readEvent();
        handleEvent();
    }
}
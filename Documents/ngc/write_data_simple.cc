status_t
writeData(const char* const buffer, size_t count) {
  Byte control = inb(CONTROLPORT);
  for (int i = 0; i < count; i++) {
    control &= ~STROBE;
    outb(DATAPORT, buffer[i]);
    outb(CONTROLPORT, control);
    L4_Sleep(SEND_DELAY);
    control |= STROBE;
    outb(CONTROLPORT, control);
    L4_Sleep(SEND_DELAY);
    ...
  }
  ...

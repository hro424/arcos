static int i IS_PERSISTENT = 0;

status_t writeData(const char* const buffer, size_t count) {
  Byte control = inb(CONTROLPORT);
  if (!Recovered) i = 0;
  while (i < count) {
    control &= ~STROBE;
    outb(DATAPORT, buffer[i]);
    outb(CONTROLPORT, control);
    i++;
    L4_Sleep(SEND_DELAY);
    control |= STROBE;
    outb(CONTROLPORT, control);
    L4_Sleep(SEND_DELAY);
    ...
  }
  ...

//include fctnl.h

int gpio_set_value(unsigned int gpio, unsigned int value)
{
        int fd, len;
        char buf[64];

        len = snprintf(buf,
	sizeof(buf),"/sys/class/gpio/gpio%d/value", gpio);

        fd = open(buf, O_WRONLY);
        if (fd < 0) {
                perror("gpio/set-value");
                return fd;
        }

        if (value)
                write(fd, "1", 2);
        else
            	write(fd, "0", 2);

        close(fd);
        return 0;
}

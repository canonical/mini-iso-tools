#!/usr/bin/python3

# debian/control files are in rfc822 format.   Python once had a `rfc822`
# module, but that was deprecated and moved to `email`.
from email.parser import Parser


def get_build_depends():
    with open('debian/control') as fp:
        headers = Parser().parse(fp)
    return [val.strip().split(' ')[0]
            for val in headers['Build-Depends'].split(',')]


def main():
    print(' '.join(get_build_depends()))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3

# Adventures in Xrandr...
# https://cgit.freedesktop.org/xorg/proto/randrproto/tree/randrproto.txt
# https://github.com/python-xlib/python-xlib/blob/master/examples/xrandr.py

import logging
import sys
import typing as t

# All .xrandr_*() methods are dynamically bound to display and (root) windows
# and can be found as Xlib.ext.randr.*() functions
import Xlib.display
import Xlib.error
if t.TYPE_CHECKING:
    import Xlib.protocol.rq
    import Xlib.X
    import Xlib.xobject.drawable

logger = logging.getLogger(__name__)


class XRandr:
    """
    display_name: if None, connects to $DISPLAY, usually ":0"
    sno: if None, uses default screen set in display_name or 0
    """
    def __init__(self, display_name: t.Optional[str] = None, sno: t.Optional[int] = None):
        self.display: Xlib.display.Display = Xlib.display.Display(display_name)
        self.sno: int = self.display.get_default_screen() if sno is None else sno
        self.root: Xlib.xobject.drawable.Window = self.display.screen(self.sno).root
        self.edid_atom: int = self.display.get_atom('EDID', only_if_exists=True)
        self.config_timestamp: int = 0
        if 'RANDR' not in self.display.extensions:
            raise Xlib.error.DisplayError("X server does not have the RANDR extension")
        self.version: Xlib.protocol.rq.ReplyRequest = self.display.xrandr_query_version()

    @property
    def outputs(self) -> t.List["Output"]:
        # noinspection PyUnresolvedReferences
        resources = self.root.xrandr_get_screen_resources_current()
        self.config_timestamp = resources.config_timestamp
        return [Output(output, self) for output in resources.outputs]


class Output:
    def __init__(self, handle: int, server: XRandr):
        self.handle: int = handle
        self.server: XRandr = server

    @property
    def info(self) -> Xlib.protocol.rq.ReplyRequest:
        """Useful attributes (for connected outputs): name, mm_width, mm_height"""
        return self.server.display.xrandr_get_output_info(
            self.handle,
            self.server.config_timestamp
        )

    @property
    def edid(self) -> bytes:
        return bytes(
            self.server.display.xrandr_get_output_property(
                output=self.handle,
                property=self.server.edid_atom,
                type=Xlib.X.AnyPropertyType,
                long_offset=0,
                # E-EDID can be up to 128 x 256 bytes blocks, each long is 4 bytes
                long_length=128 * 256 // 4,
            ).value
        )


def main(argv):
    logging.basicConfig(
        format="%(message)s",
        level=logging.DEBUG if ('-v' in argv or '--verbose' in argv) else logging.WARNING,
    )
    xrandr = XRandr()
    logger.info(
        'RANDR version %d.%d',
        xrandr.version.major_version,
        xrandr.version.minor_version,
    )
    for output in xrandr.outputs:
        logger.info(output.info.name)
        sys.stdout.buffer.write(output.edid)  # print() does not print raw bytes


if __name__ == '__main__':
    try:
        main(sys.argv)
    except Xlib.error.DisplayError as e:
        logger.error("Error connecting to X server: %s", e)

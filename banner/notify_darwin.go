package banner

/*
// Compiler flags.
#cgo CFLAGS: -Wall -x objective-c -std=gnu99 -fobjc-arc
// Linker flags.
#cgo LDFLAGS: -framework Foundation -framework Cocoa

#import "banner_darwin.h"
*/
import "C"
import "unsafe"

// Notification is an NSUserNotification.
type Notification struct {
	Title    string
	Subtitle string
	// InformativeText is the notification message.
	InformativeText string
	// ContentImage is the primary notification icon.
	ContentImage string
	// SoundName is the name of the sound that fires with a notification.
	SoundName string
	// Send notification bypassing Do Not Disturb.
	Urgent bool
}

// Send displays a NSUserNotification on macOS.
func (n *Notification) Send() {
	t := C.CString(n.Title)
	s := C.CString(n.Subtitle)
	i := C.CString(n.InformativeText)
	c := C.CString(n.ContentImage)
	sn := C.CString(n.SoundName)

	defer C.free(unsafe.Pointer(t))
	defer C.free(unsafe.Pointer(s))
	defer C.free(unsafe.Pointer(i))
	defer C.free(unsafe.Pointer(c))
	defer C.free(unsafe.Pointer(sn))
	//TODO: send bool instead of int
	if n.Urgent {
		C.Send(t, s, i, c, sn, 1)
	} else {
		C.Send(t, s, i, c, sn, 0)
	}

}

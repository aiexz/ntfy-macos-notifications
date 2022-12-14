// Source: https://github.com/variadico/noti/blob/95579985ff5aa5edca8d3105df619a229cb577f8/service/nsuser/nsuser_darwin.h

// Based on https://github.com/norio-nomura/usernotification (WTFPL, 2013)

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

@implementation NSBundle (swizle)

// Overriding bundleIdentifier works, but overriding NSUserNotificationAlertStyle does not work.
- (NSString *)__bundleIdentifier
{
    if (self == [NSBundle mainBundle]) {
        return @"com.apple.terminal";
    }

    return [self __bundleIdentifier];
}

@end

BOOL installNSBundleHook()
{
    Class c = objc_getClass("NSBundle");
    if (c) {
        method_exchangeImplementations(class_getInstanceMethod(c, @selector(bundleIdentifier)),
                                       class_getInstanceMethod(c, @selector(__bundleIdentifier)));
        return YES;
    }

    return NO;
}

@interface NotificationCenterDelegate : NSObject <NSUserNotificationCenterDelegate>

@property (nonatomic, assign) BOOL keepRunning;

@end

@implementation NotificationCenterDelegate

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification
{
    self.keepRunning = NO;
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    return YES;
}

@end

void Send(const char *title, const char *subtitle, const char *informativeText, const char *contentImage, const char *soundName, const int urgent)
{
    @autoreleasepool {
        if (!installNSBundleHook()) {
            return;
        }

        NSUserNotificationCenter *nc = [NSUserNotificationCenter defaultUserNotificationCenter];
        NotificationCenterDelegate *ncDelegate = [[NotificationCenterDelegate alloc] init];
        ncDelegate.keepRunning = YES;
        nc.delegate = ncDelegate;

        NSUserNotification *note = [[NSUserNotification alloc] init];
        note.title = [NSString stringWithUTF8String:title];
        note.subtitle = [NSString stringWithUTF8String:subtitle];
        note.informativeText = [NSString stringWithUTF8String:informativeText];
        note.soundName = [NSString stringWithUTF8String:soundName];
        note.contentImage = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:contentImage]];
		[note setValue:[NSNumber numberWithBool:urgent] forKey:@"_ignoresDoNotDisturb"];

        [nc deliverNotification:note];

		int i = 0;
        while (ncDelegate.keepRunning) {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
			i++;
			if (i > 1000) { // wish someone could explain why this is necessary
				break;
			}
        }
    }
}
/*
TODO: Fix the following code to use the new NSUserNotification API.
void BannerNotify(const char *title, const char *message, const char *sound)
{
    @autoreleasepool{
        if (!installNSBundleHook()){
            return;
        }
        UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
        content.title = [NSString stringWithUTF8String:title];
        content.subtitle = [NSString stringWithUTF8String:message];
        content.sound = [UNNotificationSound defaultSound];
        [content setValue:@YES forKeyPath:@"shouldAlwaysAlertWhileAppIsForeground"];
        UNTimeIntervalNotificationTrigger* timeTrigger = [UNTimeIntervalNotificationTrigger
                                                          triggerWithTimeInterval:0.1 repeats:NO];

        NSString *uuid = [[NSUUID UUID] UUIDString];
        UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:uuid
                                                                              content:content trigger:timeTrigger];
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter]; // crash here
        [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
                printf("Something went wrong: %@",error);
        }];

    }
}
*/
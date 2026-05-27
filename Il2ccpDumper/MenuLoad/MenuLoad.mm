#include <Foundation/NSObjCRuntime.h>
#import <ImageIO/ImageIO.h>
#import <UIKit/UIKit.h>
#include <string>
#include "Includes.h"

@interface MenuLoad ()

@property(nonatomic, strong) ImGuiDrawView *ImGuiView;
@property(nonatomic, strong) UIImage	   *CachedGifImage;

- (ImGuiDrawView *)GetImGuiDrawView;

@end

static MenuLoad *GExtraInfo;

UIButton *GInvisibleMenuButton     = nil;
UIButton *GVisibleMenuButton       = nil;
MenuInteraction *GMenuTouchView    = nil;
UITextField *GHideRecordTextField  = nil;
UIView *GHideRecordView            = nil;

@interface MenuInteraction ()
@end

@implementation MenuInteraction

// When the ImGui menu is open, capture all touches so the game receives
// nothing. When closed, pass everything through to the game.
- (BOOL)pointInside:(CGPoint)InPoint withEvent:(UIEvent *)InEvent
{
	if ([ImGuiDrawView isMenuShowing])
	{
		// Menu open: claim the full touch area so the game gets nothing.
		return [super pointInside:InPoint withEvent:InEvent];
	}
	// Menu closed: pass through so the game handles its own input.
	return NO;
}

- (void)touchesBegan:(NSSet<UITouch *> *)InTouches withEvent:(UIEvent *)InEvent
{
	[[GExtraInfo GetImGuiDrawView] updateIOWithTouchEvent:InEvent];
}

- (void)touchesMoved:(NSSet<UITouch *> *)InTouches withEvent:(UIEvent *)InEvent
{
	[[GExtraInfo GetImGuiDrawView] updateIOWithTouchEvent:InEvent];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)InTouches
			   withEvent:(UIEvent *)InEvent
{
	[[GExtraInfo GetImGuiDrawView] updateIOWithTouchEvent:InEvent];
}

- (void)touchesEnded:(NSSet<UITouch *> *)InTouches withEvent:(UIEvent *)InEvent
{
	[[GExtraInfo GetImGuiDrawView] updateIOWithTouchEvent:InEvent];
}

@end

@implementation MenuLoad

bool bIsMenuOpened = false;

- (ImGuiDrawView *)GetImGuiDrawView
{
	return _ImGuiView;
}

+ (void)load
{
	[super load];

	CallAfterSeconds(3)
	{
		settings.Load();

		GExtraInfo = [MenuLoad new];
		[GExtraInfo InitializeGestureRecognizers];
	});
}

- (void)InitializeGestureRecognizers
{
	UIView *const MainApplicationView =
		[UIApplication sharedApplication].windows[0].rootViewController.view;
	const CGRect ScreenBounds = [[UIScreen mainScreen] bounds];

	[self SetupHiddenRecordingComponents:ScreenBounds];
	[self InitializeImGuiDrawSystem];
	[self SetupTouchInteractionLayer:MainApplicationView];

	[self LoadAnimatedMenuButton:MainApplicationView];
	[self SetupInvisibleDraggableButton:MainApplicationView];
}

- (void)SetupHiddenRecordingComponents:(CGRect)InScreenBounds
{
	GHideRecordTextField				   = [[UITextField alloc] init];
	GHideRecordView						   = [[UIView alloc] initWithFrame:InScreenBounds];
	GHideRecordView.backgroundColor		   = [UIColor clearColor];
	GHideRecordView.userInteractionEnabled = YES;
	GHideRecordTextField.secureTextEntry   = YES;
	[GHideRecordView addSubview:GHideRecordTextField];

	CALayer *const TextFieldLayer = GHideRecordTextField.layer;

	if ([TextFieldLayer.sublayers.firstObject.delegate
			isKindOfClass:[UIView class]])
	{
		GHideRecordView =
			(UIView *)TextFieldLayer.sublayers.firstObject.delegate;
	}
	else
	{
		GHideRecordView = nil;
	}

	if (GHideRecordView)
	{
		[[UIApplication sharedApplication].windows.firstObject
			addSubview:GHideRecordView];
	}
}

- (void)InitializeImGuiDrawSystem
{
	if (!_ImGuiView)
	{
		_ImGuiView = [[ImGuiDrawView alloc] init];
	}

	[ImGuiDrawView showChange:NO];
	[GHideRecordView addSubview:_ImGuiView.view];
}

- (void)SetupTouchInteractionLayer:(UIView *)InMainView
{
	GMenuTouchView = [[MenuInteraction alloc] initWithFrame:InMainView.frame];
	[InMainView addSubview:GMenuTouchView];
}

- (void)LoadAnimatedMenuButton:(UIView *)InMainView
{
	NSURL *const AnimatedImageURL = [NSURL
		URLWithString:@"https://enginex.fun/img/icon.png"]; // update link for our
															// own logo

	if (self.CachedGifImage)
	{
		[self CreateVisibleMenuButton:self.CachedGifImage
						   InMainView:InMainView];
	}
	else
	{
		dispatch_async(
			dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
			  @autoreleasepool
			  {
				  NSData *const DownloadedImageData =
					  [NSData dataWithContentsOfURL:AnimatedImageURL];
				  if (DownloadedImageData)
				  {
					  UIImage *ProcessedAnimatedImage =
						  [self ProcessAnimatedImageData:DownloadedImageData];
					  if (ProcessedAnimatedImage)
					  {
						  self.CachedGifImage = ProcessedAnimatedImage;

						  dispatch_async(dispatch_get_main_queue(), ^{
							[self CreateVisibleMenuButton:ProcessedAnimatedImage
											   InMainView:InMainView];
						  });
					  }
				  }
			  }
			});
	}
}

- (void)SetupInvisibleDraggableButton:(UIView *)InMainView
{
	const CGFloat ButtonSize	 = 75.0f;
	const CGFloat HalfButtonSize = ButtonSize * 0.5f;
	const CGRect  ScreenBounds	 = [[UIScreen mainScreen] bounds];

	GInvisibleMenuButton = [UIButton buttonWithType:UIButtonTypeCustom];
	GInvisibleMenuButton.frame =
		CGRectMake(CGRectGetMidX(ScreenBounds) - HalfButtonSize,
				   CGRectGetMidY(ScreenBounds) - HalfButtonSize,
				   ButtonSize, ButtonSize);
	GInvisibleMenuButton.backgroundColor = [UIColor clearColor];

	// Tap: toggle the ImGui menu.
	[GInvisibleMenuButton addTarget:self
							 action:@selector(OnMenuButtonTapped:)
				   forControlEvents:UIControlEventTouchUpInside];

	// Drag: move the icon around the screen.
	UIPanGestureRecognizer *MenuDragGesture =
		[[UIPanGestureRecognizer alloc] initWithTarget:self
												action:@selector(OnButtonPanned:)];
	[GInvisibleMenuButton addGestureRecognizer:MenuDragGesture];

	// Add directly to the main UIWindow, on top of GHideRecordView.
	// When SCLAlertView presents modally, UIKit inserts its view above all
	// existing subviews, naturally covering this button → non-interactive
	// during alerts (as desired).
	[[UIApplication sharedApplication].windows.firstObject
		addSubview:GInvisibleMenuButton];
}

- (void)CreateVisibleMenuButton:(UIImage *)InAnimatedImage
					 InMainView:(UIView *)InMainView
{
	const CGFloat ButtonSize	 = 75.0f;
	const CGFloat HalfButtonSize = ButtonSize * 0.5f;
	const CGFloat CornerRadius	 = 25.0f;
	const CGRect  ScreenBounds	 = [[UIScreen mainScreen] bounds];

	GVisibleMenuButton = [UIButton buttonWithType:UIButtonTypeCustom];
	GVisibleMenuButton.frame =
		CGRectMake(CGRectGetMidX(ScreenBounds) - HalfButtonSize,
				   CGRectGetMidY(ScreenBounds) - HalfButtonSize,
				   ButtonSize, ButtonSize);
	GVisibleMenuButton.backgroundColor	  = [UIColor clearColor];
	GVisibleMenuButton.layer.cornerRadius = CornerRadius;
	GVisibleMenuButton.clipsToBounds	  = YES;
	GVisibleMenuButton.userInteractionEnabled = NO; // touches go to invisible button above
	[GVisibleMenuButton setImage:InAnimatedImage forState:UIControlStateNormal];

	// Insert below the invisible hit-area button so hit-testing reaches it.
	UIWindow *MainWindow = [UIApplication sharedApplication].windows.firstObject;
	NSUInteger InvisibleIdx = [MainWindow.subviews indexOfObject:GInvisibleMenuButton];
	if (InvisibleIdx != NSNotFound)
		[MainWindow insertSubview:GVisibleMenuButton atIndex:InvisibleIdx];
	else
		[MainWindow addSubview:GVisibleMenuButton];
}

- (UIImage *_Nullable)ProcessAnimatedImageData:(NSData *_Nonnull)InImageData
{
	if (!InImageData)
	{
		return nil;
	}

	CGImageSourceRef ImageSource =
		CGImageSourceCreateWithData((__bridge CFDataRef)InImageData, NULL);
	if (!ImageSource)
	{
		return nil;
	}

	const size_t			   FrameCount = CGImageSourceGetCount(ImageSource);
	NSMutableArray<UIImage *> *FrameImages =
		[NSMutableArray arrayWithCapacity:FrameCount];
	NSTimeInterval TotalAnimationDuration = 0.0f;

	for (size_t FrameIndex = 0; FrameIndex < FrameCount; FrameIndex++)
	{
		@autoreleasepool
		{
			const CGImageRef _Nullable CurrentFrameRef =
				CGImageSourceCreateImageAtIndex(ImageSource, FrameIndex, NULL);
			if (!CurrentFrameRef)
			{
				continue;
			}

			NSDictionary *FrameProperties = (__bridge_transfer NSDictionary *)
				CGImageSourceCopyPropertiesAtIndex(ImageSource, FrameIndex,
												   NULL);
			NSDictionary *GifFrameProperties = FrameProperties[(
				NSString *_Nonnull)kCGImagePropertyGIFDictionary];

			NSNumber *FrameDelayTime = GifFrameProperties[(
				NSString *_Nonnull)kCGImagePropertyGIFUnclampedDelayTime];
			if (!FrameDelayTime)
			{
				FrameDelayTime = GifFrameProperties[(
					NSString *_Nonnull)kCGImagePropertyGIFDelayTime];
			}

			const float MinFrameDuration	 = 0.011f;
			const float DefaultFrameDuration = 0.100f;
			if (FrameDelayTime.floatValue < MinFrameDuration)
			{
				FrameDelayTime = @(DefaultFrameDuration);
			}

			TotalAnimationDuration += FrameDelayTime.floatValue;

			UIImage *ProcessedFrameImage =
				[UIImage imageWithCGImage:CurrentFrameRef
									scale:[UIScreen mainScreen].scale
							  orientation:UIImageOrientationUp];
			[FrameImages addObject:ProcessedFrameImage];

			CGImageRelease(CurrentFrameRef);
		}
	}

	CFRelease(ImageSource);

	if (TotalAnimationDuration == 0.0f)
	{
		constexpr float DefaultFPS = 10.0f;
		TotalAnimationDuration	   = (1.0f / DefaultFPS) * FrameCount;
	}

	return [UIImage animatedImageWithImages:FrameImages
								   duration:TotalAnimationDuration];
}

// Called from UIControlEventTouchUpInside — sender is the UIButton itself.
- (void)OnMenuButtonTapped:(id)InSender
{
	const BOOL bCurrentMenuVisibility = [ImGuiDrawView isMenuShowing];
	if (bCurrentMenuVisibility)
		settings.Save();
	[ImGuiDrawView showChange:!bCurrentMenuVisibility];
}

- (void)OnButtonPanned:(UIPanGestureRecognizer *)InPanGesture
{
	UIView *const DraggedButton = InPanGesture.view;

	const CGPoint Translation = [InPanGesture translationInView:DraggedButton.superview];
	DraggedButton.center = CGPointMake(DraggedButton.center.x + Translation.x,
									   DraggedButton.center.y + Translation.y);
	[InPanGesture setTranslation:CGPointZero inView:DraggedButton.superview];

	[self ClampButtonToScreenBounds:(UIButton *)DraggedButton
				   WithScreenBounds:[[UIScreen mainScreen] bounds]];

	// Keep the decorative visible button centre in sync.
	GVisibleMenuButton.center = DraggedButton.center;
}

- (void)ClampButtonToScreenBounds:(UIButton *)InButton
				 WithScreenBounds:(CGRect)InScreenBounds
{
	CGPoint ClampedCenter = InButton.center;

	if (ClampedCenter.x < 0)
	{
		ClampedCenter.x = 0;
	}
	else if (ClampedCenter.x > InScreenBounds.size.width)
	{
		ClampedCenter.x = InScreenBounds.size.width;
	}

	if (ClampedCenter.y < 0)
	{
		ClampedCenter.y = 0;
	}
	else if (ClampedCenter.y > InScreenBounds.size.height)
	{
		ClampedCenter.y = InScreenBounds.size.height;
	}

	InButton.center = ClampedCenter;
}

@end

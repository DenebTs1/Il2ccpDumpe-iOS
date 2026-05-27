#include "../Menu/UserMenu.h"
#include "Includes.h"
#include "../Menu/byte_array.h"
#import <os/log.h>
#import <ImageIO/ImageIO.h>

// Font globals — defined here, declared extern in UserMenu.mm.
// Names match the example 1:1 so the ported widget code (elements::tab,
// e_elements::begin_child, etc.) finds them.
ImFont *medium         = nullptr;
ImFont *boldFont       = nullptr;
ImFont *tab_icons      = nullptr;
ImFont *logo           = nullptr;
ImFont *tab_title      = nullptr;
ImFont *tab_title_icon = nullptr;
ImFont *subtab_title   = nullptr;
ImFont *combo_arrow    = nullptr;

// Menu logo texture — loaded asynchronously, used by UserMenu to draw the icon
// in the ImGui title bar via AddImageRounded.
ImTextureID gMenuLogoTexture = nullptr;

@interface ImGuiDrawView () <MTKViewDelegate>

@property(nonatomic, strong) id<MTLDevice>		 device;
@property(nonatomic, strong) id<MTLCommandQueue> commandQueue;

@end

@implementation ImGuiDrawView

static bool MenDeal = true;

- (instancetype)initWithNibName:(nullable NSString *)nibNameOrNil
						 bundle:(nullable NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (!self)
		return nil;

	_device = MTLCreateSystemDefaultDevice();
	if (!_device)
		abort();
	_commandQueue = [_device newCommandQueue];

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	ImGuiStyle &style = ImGui::GetStyle();

	io.Fonts->Clear();

	// Font setup — copied 1:1 from Example/src/gui/gui.cpp::SetupMenu.
	ImFontConfig font_config;
	font_config.PixelSnapH        = false;
	font_config.OversampleH       = 5;
	font_config.OversampleV       = 5;
	font_config.RasterizerMultiply = 1.2f;

	static const ImWchar ranges[] = {
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0xE000, 0xE226, // icons
		0,
	};
	font_config.GlyphRanges = ranges;

	medium         = io.Fonts->AddFontFromMemoryTTF(PTRootUIMedium, sizeof(PTRootUIMedium), 15.0f, &font_config, ranges);
	boldFont       = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold,   sizeof(PTRootUIBold),   15.0f, &font_config, ranges);
	tab_icons      = io.Fonts->AddFontFromMemoryTTF(clarityfont,    sizeof(clarityfont),    15.0f, &font_config, ranges);
	logo           = io.Fonts->AddFontFromMemoryTTF(clarityfont,    sizeof(clarityfont),    21.0f, &font_config, ranges);
	tab_title      = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold,   sizeof(PTRootUIBold),   19.0f, &font_config, ranges);
	tab_title_icon = io.Fonts->AddFontFromMemoryTTF(clarityfont,    sizeof(clarityfont),    18.0f, &font_config, ranges);
	subtab_title   = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold,   sizeof(PTRootUIBold),   15.0f, &font_config, ranges);
	combo_arrow    = io.Fonts->AddFontFromMemoryTTF(combo,          sizeof(combo),          9.0f,  &font_config, ranges);

	if (!medium)
		io.Fonts->AddFontDefault();

	ImGui_ImplMetal_Init(_device);

	// Kick off async download of the menu icon (same URL as MenuLoad uses)
	// and create a Metal texture for ImGui rendering.
	[self loadMenuLogoTexture];

	return self;
}

+ (void)showChange:(BOOL)open
{
	MenDeal = open;
	UserMenu::GetInstance().SetVisible(open);
}

+ (BOOL)isMenuShowing
{
	return MenDeal;
}

- (MTKView *)mtkView
{
	return (MTKView *)self.view;
}

- (void)loadView
{
	CGFloat w = [UIApplication sharedApplication]
					.windows[0]
					.rootViewController.view.frame.size.width;
	CGFloat h = [UIApplication sharedApplication]
					.windows[0]
					.rootViewController.view.frame.size.height;
	self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, w, h)];
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	self.mtkView.device				   = self.device;
	self.mtkView.delegate			   = self;
	self.mtkView.paused				   = NO;
	self.mtkView.enableSetNeedsDisplay = NO;
	self.mtkView.preferredFramesPerSecond =
		UIScreen.mainScreen.maximumFramesPerSecond ?: 60;
	self.mtkView.clearColor		 = MTLClearColorMake(0, 0, 0, 0);
	self.mtkView.backgroundColor = [UIColor colorWithRed:0
												   green:0
													blue:0
												   alpha:0];
	self.mtkView.clipsToBounds	 = YES;
}

- (void)updateIOWithTouchEvent:(UIEvent *)event
{
	UITouch *anyTouch	   = event.allTouches.anyObject;
	CGPoint	 touchLocation = [anyTouch locationInView:self.view];
	ImGuiIO &io			   = ImGui::GetIO();

	// Queue position FIRST so ImGui registers hover at the new location before
	// it processes the button state. Setting MousePos and MouseDown together
	// (legacy API) made ImGui see "cursor appeared while already pressed",
	// which it treats as a drag — so the first tap was lost and a second tap
	// was needed to actually click.
	io.AddMousePosEvent(touchLocation.x, touchLocation.y);

	BOOL hasActiveTouch = NO;
	for (UITouch *touch in event.allTouches)
	{
		if (touch.phase != UITouchPhaseEnded &&
			touch.phase != UITouchPhaseCancelled)
		{
			hasActiveTouch = YES;
			break;
		}
	}
	io.AddMouseButtonEvent(0, hasActiveTouch);
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	[self updateIOWithTouchEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	[self updateIOWithTouchEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	[self updateIOWithTouchEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	[self updateIOWithTouchEvent:event];
}

- (void)drawInMTKView:(MTKView *)view
{
	GHideRecordTextField.secureTextEntry = settings.StreamerMode;

	ImGuiIO &io = ImGui::GetIO();
	if (UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad)
	{
		io.FontGlobalScale = 1.3f;
	}

	io.DisplaySize.x = view.bounds.size.width;
	io.DisplaySize.y = view.bounds.size.height;

	CGFloat framebufferScale =
		view.window.screen.nativeScale ?: UIScreen.mainScreen.nativeScale;
	io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);
	io.DeltaTime			   = 1 / float(view.preferredFramesPerSecond ?: 30);

	id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

	if (MenDeal)
	{
		[self.view setUserInteractionEnabled:YES];
		[self.view.superview setUserInteractionEnabled:YES];
		[GMenuTouchView setUserInteractionEnabled:YES];
	}
	else
	{
		[self.view setUserInteractionEnabled:NO];
		[self.view.superview setUserInteractionEnabled:NO];
		[GMenuTouchView setUserInteractionEnabled:NO];
	}

	MTLRenderPassDescriptor *renderPassDescriptor =
		view.currentRenderPassDescriptor;
	if (renderPassDescriptor == nil)
	{
		[commandBuffer commit];
		return;
	}

	if (renderPassDescriptor != nil)
	{
		id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer
			renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[renderEncoder pushDebugGroup:@"Dear ImGui Rendering"];

		ImGui_ImplMetal_NewFrame(renderPassDescriptor);
		ImGui::NewFrame();

		// Menu Draw
		if (MenDeal)
		{
			UserMenu::GetInstance().RenderMenu();
		}

		ImGui::Render();
		ImDrawData *draw_data = ImGui::GetDrawData();
		ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);

		[renderEncoder popDebugGroup];
		[renderEncoder endEncoding];

		[commandBuffer presentDrawable:view.currentDrawable];
	}

	[commandBuffer commit];
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{
}

- (void)loadMenuLogoTexture
{
	NSURL *iconURL = [NSURL URLWithString:@"https://enginex.fun/img/icon.png"];
	id<MTLDevice> device = _device;

	dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
		@autoreleasepool {
			NSData *data = [NSData dataWithContentsOfURL:iconURL];
			if (!data) return;

			CGImageSourceRef src = CGImageSourceCreateWithData((__bridge CFDataRef)data, NULL);
			if (!src) return;

			CGImageRef cgImg = CGImageSourceCreateImageAtIndex(src, 0, NULL);
			CFRelease(src);
			if (!cgImg) return;

			size_t w = CGImageGetWidth(cgImg);
			size_t h = CGImageGetHeight(cgImg);

			// Render into a 32-bit RGBA buffer
			NSMutableData *rgba = [NSMutableData dataWithLength:w * h * 4];
			CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
			CGContextRef ctx = CGBitmapContextCreate(
				rgba.mutableBytes, w, h, 8, w * 4, cs,
				kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
			CGColorSpaceRelease(cs);
			CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), cgImg);
			CGContextRelease(ctx);
			CGImageRelease(cgImg);

			// Create Metal texture
			MTLTextureDescriptor *desc = [MTLTextureDescriptor
				texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
				width:w height:h mipmapped:NO];
			desc.usage = MTLTextureUsageShaderRead;
			desc.storageMode = MTLStorageModeShared;

			id<MTLTexture> tex = [device newTextureWithDescriptor:desc];
			[tex replaceRegion:MTLRegionMake2D(0, 0, w, h)
				  mipmapLevel:0
					withBytes:rgba.bytes
				  bytesPerRow:w * 4];

			// Publish on main thread so ImGui picks it up safely
			dispatch_async(dispatch_get_main_queue(), ^{
				gMenuLogoTexture = (__bridge_retained void *)tex;
			});
		}
	});
}

@end

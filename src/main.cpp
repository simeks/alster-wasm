#include <emscripten.h>
#include <glfw/glfw3.h>
#include <webgpu/webgpu.h>

#include <cassert>
#include <cstdio>

extern "C" {
extern void hello();
}

GLFWwindow* window_ = nullptr;

WGPUInstance instance_ = nullptr;
WGPUSurface surface_ = nullptr;
WGPUSwapChain swapchain_ = nullptr;
WGPUAdapter adapter_ = nullptr;
WGPUDevice device_ = nullptr;
WGPUQueue queue_ = nullptr;

WGPUShaderModule vs_module_ = nullptr;
WGPUShaderModule fs_module_ = nullptr;
WGPUPipelineLayout pipeline_layout_ = nullptr;
WGPURenderPipeline pipeline_ = nullptr;

const char* vs_src = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(in_vertex_index) - 1);
    let y = f32(i32(in_vertex_index & 1u) * 2 - 1);
    return vec4<f32>(x, y, 0.0, 1.0);
}
)";

const char* fs_src = R"(
@fragment
fn fs_main() -> @location(0) vec4<f32> {
    return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}
)";

WGPUShaderModule create_shader_module(WGPUDevice device, char const* src) {
  WGPUShaderModuleWGSLDescriptor wgsl_desc{
    .chain = {.sType = WGPUSType_ShaderModuleWGSLDescriptor},
    .code = src,
  };
  WGPUShaderModuleDescriptor desc{
    .nextInChain = (const WGPUChainedStruct*)&wgsl_desc,
  };
  return wgpuDeviceCreateShaderModule(device, &desc);
}

void setup_pipeline() {
  WGPUShaderModuleWGSLDescriptor vs_desc{
    .code = vs_src,
  };
  vs_module_ = create_shader_module(device_, vs_src);
  fs_module_ = create_shader_module(device_, fs_src);

  WGPUPipelineLayoutDescriptor layout_desc{};
  pipeline_layout_ = wgpuDeviceCreatePipelineLayout(device_, &layout_desc);

  WGPUColorTargetState target_desc{
    .format = WGPUTextureFormat_BGRA8Unorm,
    .writeMask = WGPUColorWriteMask_All,
  };

  WGPUFragmentState fragment_state{
    .module = fs_module_,
    .entryPoint = "fs_main",
    .targetCount = 1,
    .targets = &target_desc,
  };
  WGPURenderPipelineDescriptor pipeline_desc{
    .layout = pipeline_layout_,
    .vertex =
      {
        .module = vs_module_,
        .entryPoint = "vs_main",
      },
    .primitive =
      {
        .topology = WGPUPrimitiveTopology_TriangleList,
      },
    .multisample =
      {
        .count = 1,
        .mask = 0xFFFFFFFF,
      },
    .fragment = &fragment_state,
  };
  pipeline_ = wgpuDeviceCreateRenderPipeline(device_, &pipeline_desc);
  assert(pipeline_);
}

void init() {
  WGPUInstanceDescriptor instance_desc = {};
  instance_ = wgpuCreateInstance(&instance_desc);
  assert(instance_);

  assert(glfwInit());

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ = glfwCreateWindow(640, 480, "Hello, World!", nullptr, nullptr);
  assert(window_);

  WGPUSurfaceDescriptorFromCanvasHTMLSelector canvas_desc{
    .chain = {.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector},
    .selector = "canvas",
  };
  WGPUSurfaceDescriptor surface_desc{
    .nextInChain = &canvas_desc.chain,
  };
  surface_ = wgpuInstanceCreateSurface(instance_, &surface_desc);
  assert(surface_);

  WGPURequestAdapterOptions adapter_options = {
    .compatibleSurface = surface_,
  };

  bool adapter_request_complete = false;
  wgpuInstanceRequestAdapter(
    instance_, &adapter_options,
    [](
      WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* msg,
      void* user_data) {
      if (status == WGPURequestAdapterStatus_Success) {
        adapter_ = adapter;
      }
      *reinterpret_cast<bool*>(user_data) = true;
    },
    &adapter_request_complete);
  // Request is async so we need to wait for the adapter to be set.
  while (!adapter_request_complete) {
    emscripten_sleep(100);
  }
  assert(adapter_);

  bool device_request_complete = false;
  wgpuAdapterRequestDevice(
    adapter_, nullptr,
    [](
      WGPURequestDeviceStatus status, WGPUDevice device, char const*,
      void* user_data) {
      if (status == WGPURequestDeviceStatus_Success) {
        device_ = device;
      }
      *reinterpret_cast<bool*>(user_data) = true;
    },
    &device_request_complete);
  // same story
  while (!device_request_complete) {
    emscripten_sleep(100);
  }
  assert(device_);

  queue_ = wgpuDeviceGetQueue(device_);
  assert(queue_);

  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);

  WGPUSwapChainDescriptor swapchain_desc{
    .usage = WGPUTextureUsage_RenderAttachment,
    .format = WGPUTextureFormat_BGRA8Unorm,
    .width = uint32_t(width),
    .height = uint32_t(height),
    .presentMode = WGPUPresentMode_Fifo,
  };

  swapchain_ = wgpuDeviceCreateSwapChain(device_, surface_, &swapchain_desc);
  assert(swapchain_);

  setup_pipeline();
}

void frame() {
  WGPUTextureView target_view = wgpuSwapChainGetCurrentTextureView(swapchain_);
  assert(target_view);

  WGPUCommandEncoder command_encoder =
    wgpuDeviceCreateCommandEncoder(device_, nullptr);
  assert(command_encoder);

  WGPURenderPassColorAttachment color_attachment{
    .view = target_view,
    .loadOp = WGPULoadOp_Clear,
    .storeOp = WGPUStoreOp_Store,
    .clearValue = {.r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
  };
  WGPURenderPassDescriptor pass_desc{
    .colorAttachmentCount = 1,
    .colorAttachments = &color_attachment,
  };
  WGPURenderPassEncoder pass_encoder =
    wgpuCommandEncoderBeginRenderPass(command_encoder, &pass_desc);
  assert(pass_encoder);

  wgpuRenderPassEncoderSetPipeline(pass_encoder, pipeline_);
  wgpuRenderPassEncoderDraw(pass_encoder, 3, 1, 0, 0);
  wgpuRenderPassEncoderEnd(pass_encoder);

  WGPUCommandBuffer command_buffer =
    wgpuCommandEncoderFinish(command_encoder, nullptr);
  assert(command_buffer);

  wgpuQueueSubmit(queue_, 1, &command_buffer);

  wgpuSwapChainPresent(swapchain_);

  wgpuCommandBufferRelease(command_buffer);
  wgpuRenderPassEncoderRelease(pass_encoder);
  wgpuCommandEncoderRelease(command_encoder);
  wgpuTextureViewRelease(target_view);
}

int main() {
  printf("Hello, World!\n");
  hello();
  init();

  emscripten_set_main_loop_arg([](void*) { frame(); }, nullptr, 0, 1);

  return 0;
}

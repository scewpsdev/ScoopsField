#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <video/khronos/vulkan/vulkan.h>

#define SDL_internal_h_
#define SDL_sysvideo_h_

#undef MAX_TEXTURE_SAMPLERS_PER_STAGE
#undef MAX_STORAGE_TEXTURES_PER_STAGE
#undef MAX_STORAGE_BUFFERS_PER_STAGE 
#undef MAX_UNIFORM_BUFFERS_PER_STAGE 
#undef MAX_COMPUTE_WRITE_TEXTURES    
#undef MAX_COMPUTE_WRITE_BUFFERS     
#undef UNIFORM_BUFFER_SIZE           
#undef MAX_VERTEX_BUFFERS            
#undef MAX_VERTEX_ATTRIBUTES         
#undef MAX_COLOR_TARGET_BINDINGS     
#undef MAX_PRESENT_COUNT             
#undef MAX_FRAMES_IN_FLIGHT          

struct SDL_VideoDevice;

#include <gpu/SDL_sysgpu.h>


struct SDL_HashTable;

typedef struct VulkanExtensions
{
    // These extensions are required!

    // Globally supported
    Uint8 KHR_swapchain;
    // Core since 1.1, needed for negative VkViewport::height
    Uint8 KHR_maintenance1;

    // These extensions are optional!

    // Core since 1.2, but requires annoying paperwork to implement
    Uint8 KHR_driver_properties;
    // Only required for special implementations (i.e. MoltenVK)
    Uint8 KHR_portability_subset;
    // Only required to detect devices using Dozen D3D12 driver
    Uint8 MSFT_layered_driver;
    // Only required for decoding HDR ASTC textures
    Uint8 EXT_texture_compression_astc_hdr;
} VulkanExtensions;

// Defines

#define SMALL_ALLOCATION_THRESHOLD    2097152  // 2   MiB
#define SMALL_ALLOCATION_SIZE         16777216 // 16  MiB
#define LARGE_ALLOCATION_INCREMENT    67108864 // 64  MiB
#define MAX_UBO_SECTION_SIZE          4096     // 4   KiB
#define DESCRIPTOR_POOL_SIZE          128
#define WINDOW_PROPERTY_DATA          "SDL_GPUVulkanWindowPropertyData"

#define IDENTITY_SWIZZLE               \
    {                                  \
        VK_COMPONENT_SWIZZLE_IDENTITY, \
        VK_COMPONENT_SWIZZLE_IDENTITY, \
        VK_COMPONENT_SWIZZLE_IDENTITY, \
        VK_COMPONENT_SWIZZLE_IDENTITY  \
    }

// Conversions

// Structures

typedef struct VulkanMemoryAllocation VulkanMemoryAllocation;
typedef struct VulkanBuffer VulkanBuffer;
typedef struct VulkanBufferContainer VulkanBufferContainer;
typedef struct VulkanUniformBuffer VulkanUniformBuffer;
typedef struct VulkanTexture VulkanTexture;
typedef struct VulkanTextureContainer VulkanTextureContainer;

typedef struct VulkanFenceHandle
{
    VkFence fence;
    SDL_AtomicInt referenceCount;
} VulkanFenceHandle;

// Memory Allocation

typedef struct VulkanMemoryFreeRegion
{
    VulkanMemoryAllocation* allocation;
    VkDeviceSize offset;
    VkDeviceSize size;
    Uint32 allocationIndex;
    Uint32 sortedIndex;
} VulkanMemoryFreeRegion;

typedef struct VulkanMemoryUsedRegion
{
    VulkanMemoryAllocation* allocation;
    VkDeviceSize offset;
    VkDeviceSize size;
    VkDeviceSize resourceOffset; // differs from offset based on alignment
    VkDeviceSize resourceSize;   // differs from size based on alignment
    VkDeviceSize alignment;
    Uint8 isBuffer;
    union
    {
        VulkanBuffer* vulkanBuffer;
        VulkanTexture* vulkanTexture;
    };
} VulkanMemoryUsedRegion;

typedef struct VulkanMemorySubAllocator
{
    Uint32 memoryTypeIndex;
    VulkanMemoryAllocation** allocations;
    Uint32 allocationCount;
    VulkanMemoryFreeRegion** sortedFreeRegions;
    Uint32 sortedFreeRegionCount;
    Uint32 sortedFreeRegionCapacity;
} VulkanMemorySubAllocator;

struct VulkanMemoryAllocation
{
    VulkanMemorySubAllocator* allocator;
    VkDeviceMemory memory;
    VkDeviceSize size;
    VulkanMemoryUsedRegion** usedRegions;
    Uint32 usedRegionCount;
    Uint32 usedRegionCapacity;
    VulkanMemoryFreeRegion** freeRegions;
    Uint32 freeRegionCount;
    Uint32 freeRegionCapacity;
    Uint8 availableForAllocation;
    VkDeviceSize freeSpace;
    VkDeviceSize usedSpace;
    Uint8* mapPointer;
    SDL_Mutex* memoryLock;
};

typedef struct VulkanMemoryAllocator
{
    VulkanMemorySubAllocator subAllocators[VK_MAX_MEMORY_TYPES];
} VulkanMemoryAllocator;

// Memory structures

typedef enum VulkanBufferType
{
    VULKAN_BUFFER_TYPE_GPU,
    VULKAN_BUFFER_TYPE_UNIFORM,
    VULKAN_BUFFER_TYPE_TRANSFER
} VulkanBufferType;

struct VulkanBuffer
{
    VulkanBufferContainer* container;
    Uint32 containerIndex;

    VkBuffer buffer;
    VulkanMemoryUsedRegion* usedRegion;

    // Needed for uniforms and defrag
    VulkanBufferType type;
    SDL_GPUBufferUsageFlags usage;
    VkDeviceSize size;

    SDL_AtomicInt referenceCount;
    bool transitioned;
    bool markedForDestroy; // so that defrag doesn't double-free
    VulkanUniformBuffer* uniformBufferForDefrag;
};

struct VulkanBufferContainer
{
    VulkanBuffer* activeBuffer;

    VulkanBuffer** buffers;
    Uint32 bufferCapacity;
    Uint32 bufferCount;

    bool dedicated;
    char* debugName;
};

// Renderer Structure

typedef struct QueueFamilyIndices
{
    Uint32 graphicsFamily;
    Uint32 presentFamily;
    Uint32 computeFamily;
    Uint32 transferFamily;
} QueueFamilyIndices;

typedef struct VulkanSampler
{
    VkSampler sampler;
    SDL_AtomicInt referenceCount;
} VulkanSampler;

typedef struct VulkanShader
{
    VkShaderModule shaderModule;
    char* entrypointName;
    SDL_GPUShaderStage stage;
    Uint32 numSamplers;
    Uint32 numStorageTextures;
    Uint32 numStorageBuffers;
    Uint32 numUniformBuffers;
    SDL_AtomicInt referenceCount;
} VulkanShader;

/* Textures are made up of individual subresources.
 * This helps us barrier the resource efficiently.
 */
typedef struct VulkanTextureSubresource
{
    VulkanTexture* parent;
    Uint32 layer;
    Uint32 level;

    VkImageView* renderTargetViews; // One render target view per depth slice
    VkImageView computeWriteView;
    VkImageView depthStencilView;
} VulkanTextureSubresource;

struct VulkanTexture
{
    VulkanTextureContainer* container;
    Uint32 containerIndex;

    VulkanMemoryUsedRegion* usedRegion;

    VkImage image;
    VkImageView fullView; // used for samplers and storage reads
    VkComponentMapping swizzle;
    VkImageAspectFlags aspectFlags;
    Uint32 depth; // used for cleanup only

    // FIXME: It'd be nice if we didn't have to have this on the texture...
    SDL_GPUTextureUsageFlags usage; // used for defrag transitions only.

    Uint32 subresourceCount;
    VulkanTextureSubresource* subresources;

    bool markedForDestroy; // so that defrag doesn't double-free
    SDL_AtomicInt referenceCount;
};

struct VulkanTextureContainer
{
    TextureCommonHeader header;

    VulkanTexture* activeTexture;

    Uint32 textureCapacity;
    Uint32 textureCount;
    VulkanTexture** textures;

    char* debugName;
    bool canBeCycled;
};

typedef enum VulkanBufferUsageMode
{
    VULKAN_BUFFER_USAGE_MODE_COPY_SOURCE,
    VULKAN_BUFFER_USAGE_MODE_COPY_DESTINATION,
    VULKAN_BUFFER_USAGE_MODE_VERTEX_READ,
    VULKAN_BUFFER_USAGE_MODE_INDEX_READ,
    VULKAN_BUFFER_USAGE_MODE_INDIRECT,
    VULKAN_BUFFER_USAGE_MODE_GRAPHICS_STORAGE_READ,
    VULKAN_BUFFER_USAGE_MODE_COMPUTE_STORAGE_READ,
    VULKAN_BUFFER_USAGE_MODE_COMPUTE_STORAGE_READ_WRITE,
} VulkanBufferUsageMode;

typedef enum VulkanTextureUsageMode
{
    VULKAN_TEXTURE_USAGE_MODE_UNINITIALIZED,
    VULKAN_TEXTURE_USAGE_MODE_COPY_SOURCE,
    VULKAN_TEXTURE_USAGE_MODE_COPY_DESTINATION,
    VULKAN_TEXTURE_USAGE_MODE_SAMPLER,
    VULKAN_TEXTURE_USAGE_MODE_GRAPHICS_STORAGE_READ,
    VULKAN_TEXTURE_USAGE_MODE_COMPUTE_STORAGE_READ,
    VULKAN_TEXTURE_USAGE_MODE_COMPUTE_STORAGE_READ_WRITE,
    VULKAN_TEXTURE_USAGE_MODE_COLOR_ATTACHMENT,
    VULKAN_TEXTURE_USAGE_MODE_DEPTH_STENCIL_ATTACHMENT,
    VULKAN_TEXTURE_USAGE_MODE_PRESENT
} VulkanTextureUsageMode;

typedef enum VulkanUniformBufferStage
{
    VULKAN_UNIFORM_BUFFER_STAGE_VERTEX,
    VULKAN_UNIFORM_BUFFER_STAGE_FRAGMENT,
    VULKAN_UNIFORM_BUFFER_STAGE_COMPUTE
} VulkanUniformBufferStage;

typedef struct VulkanFramebuffer
{
    VkFramebuffer framebuffer;
    SDL_AtomicInt referenceCount;
} VulkanFramebuffer;

typedef struct WindowData
{
    SDL_Window* window;
    SDL_GPUSwapchainComposition swapchainComposition;
    SDL_GPUPresentMode presentMode;
    bool needsSwapchainRecreate;
    Uint32 swapchainCreateWidth;
    Uint32 swapchainCreateHeight;

    // Window surface
    VkSurfaceKHR surface;

    // Swapchain for window surface
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkColorSpaceKHR colorSpace;
    VkComponentMapping swapchainSwizzle;
    bool usingFallbackFormat;

    // Swapchain images
    VulkanTextureContainer* textureContainers; // use containers so that swapchain textures can use the same API as other textures
    Uint32 imageCount;
    Uint32 width;
    Uint32 height;

    // Synchronization primitives
    VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore* renderFinishedSemaphore;
    SDL_GPUFence* inFlightFences[MAX_FRAMES_IN_FLIGHT];

    Uint32 frameCounter;
} WindowData;

typedef struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    Uint32 formatsLength;
    VkPresentModeKHR* presentModes;
    Uint32 presentModesLength;
} SwapchainSupportDetails;

typedef struct VulkanPresentData
{
    WindowData* windowData;
    Uint32 swapchainImageIndex;
} VulkanPresentData;

struct VulkanUniformBuffer
{
    VulkanBuffer* buffer;
    Uint32 drawOffset;
    Uint32 writeOffset;
};

typedef struct VulkanDescriptorInfo
{
    VkDescriptorType descriptorType;
    VkShaderStageFlagBits stageFlag;
} VulkanDescriptorInfo;

typedef struct DescriptorSetPool
{
    // It's a pool... of pools!!!
    Uint32 poolCount;
    VkDescriptorPool* descriptorPools;

    // We'll just manage the descriptor sets ourselves instead of freeing the sets
    VkDescriptorSet* descriptorSets;
    Uint32 descriptorSetCount;
    Uint32 descriptorSetIndex;
} DescriptorSetPool;

// A command buffer acquires a cache at command buffer acquisition time
typedef struct DescriptorSetCache
{
    // Pools are indexed by DescriptorSetLayoutID which increases monotonically
    // There's only a certain number of maximum layouts possible since we de-duplicate them.
    DescriptorSetPool* pools;
    Uint32 poolCount;
} DescriptorSetCache;

typedef struct DescriptorSetLayoutHashTableKey
{
    VkShaderStageFlagBits shaderStage;
    // Category 1: read resources
    Uint32 samplerCount;
    Uint32 storageBufferCount;
    Uint32 storageTextureCount;
    // Category 2: write resources
    Uint32 writeStorageBufferCount;
    Uint32 writeStorageTextureCount;
    // Category 3: uniform buffers
    Uint32 uniformBufferCount;
} DescriptorSetLayoutHashTableKey;

typedef uint32_t DescriptorSetLayoutID;

typedef struct DescriptorSetLayout
{
    DescriptorSetLayoutID ID;
    VkDescriptorSetLayout descriptorSetLayout;

    // Category 1: read resources
    Uint32 samplerCount;
    Uint32 storageBufferCount;
    Uint32 storageTextureCount;
    // Category 2: write resources
    Uint32 writeStorageBufferCount;
    Uint32 writeStorageTextureCount;
    // Category 3: uniform buffers
    Uint32 uniformBufferCount;
} DescriptorSetLayout;

typedef struct GraphicsPipelineResourceLayoutHashTableKey
{
    Uint32 vertexSamplerCount;
    Uint32 vertexStorageTextureCount;
    Uint32 vertexStorageBufferCount;
    Uint32 vertexUniformBufferCount;

    Uint32 fragmentSamplerCount;
    Uint32 fragmentStorageTextureCount;
    Uint32 fragmentStorageBufferCount;
    Uint32 fragmentUniformBufferCount;
} GraphicsPipelineResourceLayoutHashTableKey;

typedef struct VulkanGraphicsPipelineResourceLayout
{
    VkPipelineLayout pipelineLayout;

    /*
     * Descriptor set layout is as follows:
     * 0: vertex resources
     * 1: vertex uniform buffers
     * 2: fragment resources
     * 3: fragment uniform buffers
     */
    DescriptorSetLayout* descriptorSetLayouts[4];

    Uint32 vertexSamplerCount;
    Uint32 vertexStorageTextureCount;
    Uint32 vertexStorageBufferCount;
    Uint32 vertexUniformBufferCount;

    Uint32 fragmentSamplerCount;
    Uint32 fragmentStorageTextureCount;
    Uint32 fragmentStorageBufferCount;
    Uint32 fragmentUniformBufferCount;
} VulkanGraphicsPipelineResourceLayout;

typedef struct VulkanGraphicsPipeline
{
    GraphicsPipelineCommonHeader header;

    VkPipeline pipeline;
    SDL_GPUPrimitiveType primitiveType;

    VulkanGraphicsPipelineResourceLayout* resourceLayout;

    VulkanShader* vertexShader;
    VulkanShader* fragmentShader;

    SDL_AtomicInt referenceCount;
} VulkanGraphicsPipeline;

typedef struct ComputePipelineResourceLayoutHashTableKey
{
    Uint32 samplerCount;
    Uint32 readonlyStorageTextureCount;
    Uint32 readonlyStorageBufferCount;
    Uint32 readWriteStorageTextureCount;
    Uint32 readWriteStorageBufferCount;
    Uint32 uniformBufferCount;
} ComputePipelineResourceLayoutHashTableKey;

typedef struct VulkanComputePipelineResourceLayout
{
    VkPipelineLayout pipelineLayout;

    /*
     * Descriptor set layout is as follows:
     * 0: samplers, then read-only textures, then read-only buffers
     * 1: write-only textures, then write-only buffers
     * 2: uniform buffers
     */
    DescriptorSetLayout* descriptorSetLayouts[3];

    Uint32 numSamplers;
    Uint32 numReadonlyStorageTextures;
    Uint32 numReadonlyStorageBuffers;
    Uint32 numReadWriteStorageTextures;
    Uint32 numReadWriteStorageBuffers;
    Uint32 numUniformBuffers;
} VulkanComputePipelineResourceLayout;

typedef struct VulkanComputePipeline
{
    ComputePipelineCommonHeader header;

    VkShaderModule shaderModule;
    VkPipeline pipeline;
    VulkanComputePipelineResourceLayout* resourceLayout;
    SDL_AtomicInt referenceCount;
} VulkanComputePipeline;

typedef struct RenderPassColorTargetDescription
{
    VkFormat format;
    SDL_GPULoadOp loadOp;
    SDL_GPUStoreOp storeOp;
} RenderPassColorTargetDescription;

typedef struct RenderPassDepthStencilTargetDescription
{
    VkFormat format;
    SDL_GPULoadOp loadOp;
    SDL_GPUStoreOp storeOp;
    SDL_GPULoadOp stencilLoadOp;
    SDL_GPUStoreOp stencilStoreOp;
} RenderPassDepthStencilTargetDescription;

typedef struct CommandPoolHashTableKey
{
    SDL_ThreadID threadID;
} CommandPoolHashTableKey;

typedef struct RenderPassHashTableKey
{
    RenderPassColorTargetDescription colorTargetDescriptions[MAX_COLOR_TARGET_BINDINGS];
    Uint32 numColorTargets;
    VkFormat resolveTargetFormats[MAX_COLOR_TARGET_BINDINGS];
    Uint32 numResolveTargets;
    RenderPassDepthStencilTargetDescription depthStencilTargetDescription;
    VkSampleCountFlagBits sampleCount;
} RenderPassHashTableKey;

typedef struct VulkanRenderPassHashTableValue
{
    VkRenderPass handle;
} VulkanRenderPassHashTableValue;

typedef struct FramebufferHashTableKey
{
    VkImageView colorAttachmentViews[MAX_COLOR_TARGET_BINDINGS];
    Uint32 numColorTargets;
    VkImageView resolveAttachmentViews[MAX_COLOR_TARGET_BINDINGS];
    Uint32 numResolveAttachments;
    VkImageView depthStencilAttachmentView;
    Uint32 width;
    Uint32 height;
} FramebufferHashTableKey;

// Command structures

typedef struct VulkanFencePool
{
    SDL_Mutex* lock;

    VulkanFenceHandle** availableFences;
    Uint32 availableFenceCount;
    Uint32 availableFenceCapacity;
} VulkanFencePool;

typedef struct VulkanCommandPool VulkanCommandPool;

typedef struct VulkanRenderer VulkanRenderer;

typedef struct VulkanCommandBuffer
{
    CommandBufferCommonHeader common;
    VulkanRenderer* renderer;

    VkCommandBuffer commandBuffer;
    VulkanCommandPool* commandPool;

    VulkanPresentData* presentDatas;
    Uint32 presentDataCount;
    Uint32 presentDataCapacity;

    VkSemaphore* waitSemaphores;
    Uint32 waitSemaphoreCount;
    Uint32 waitSemaphoreCapacity;

    VkSemaphore* signalSemaphores;
    Uint32 signalSemaphoreCount;
    Uint32 signalSemaphoreCapacity;

    VulkanComputePipeline* currentComputePipeline;
    VulkanGraphicsPipeline* currentGraphicsPipeline;

    // Keep track of resources transitioned away from their default state to barrier them on pass end

    VulkanTextureSubresource* colorAttachmentSubresources[MAX_COLOR_TARGET_BINDINGS];
    Uint32 colorAttachmentSubresourceCount;
    VulkanTextureSubresource* resolveAttachmentSubresources[MAX_COLOR_TARGET_BINDINGS];
    Uint32 resolveAttachmentSubresourceCount;

    VulkanTextureSubresource* depthStencilAttachmentSubresource; // may be NULL

    // Dynamic state

    VkViewport currentViewport;
    VkRect2D currentScissor;
    float blendConstants[4];
    Uint8 stencilRef;

    // Resource bind state

    DescriptorSetCache* descriptorSetCache; // acquired when command buffer is acquired

    bool needNewVertexResourceDescriptorSet;
    bool needNewVertexUniformDescriptorSet;
    bool needNewVertexUniformOffsets;
    bool needNewFragmentResourceDescriptorSet;
    bool needNewFragmentUniformDescriptorSet;
    bool needNewFragmentUniformOffsets;

    bool needNewComputeReadOnlyDescriptorSet;
    bool needNewComputeReadWriteDescriptorSet;
    bool needNewComputeUniformDescriptorSet;
    bool needNewComputeUniformOffsets;

    VkDescriptorSet vertexResourceDescriptorSet;
    VkDescriptorSet vertexUniformDescriptorSet;
    VkDescriptorSet fragmentResourceDescriptorSet;
    VkDescriptorSet fragmentUniformDescriptorSet;

    VkDescriptorSet computeReadOnlyDescriptorSet;
    VkDescriptorSet computeReadWriteDescriptorSet;
    VkDescriptorSet computeUniformDescriptorSet;

    VkBuffer vertexBuffers[MAX_VERTEX_BUFFERS];
    VkDeviceSize vertexBufferOffsets[MAX_VERTEX_BUFFERS];
    Uint32 vertexBufferCount;
    bool needVertexBufferBind;

    VkImageView vertexSamplerTextureViewBindings[MAX_TEXTURE_SAMPLERS_PER_STAGE];
    VkSampler vertexSamplerBindings[MAX_TEXTURE_SAMPLERS_PER_STAGE];
    VkImageView vertexStorageTextureViewBindings[MAX_STORAGE_TEXTURES_PER_STAGE];
    VkBuffer vertexStorageBufferBindings[MAX_STORAGE_BUFFERS_PER_STAGE];

    VkImageView fragmentSamplerTextureViewBindings[MAX_TEXTURE_SAMPLERS_PER_STAGE];
    VkSampler fragmentSamplerBindings[MAX_TEXTURE_SAMPLERS_PER_STAGE];
    VkImageView fragmentStorageTextureViewBindings[MAX_STORAGE_TEXTURES_PER_STAGE];
    VkBuffer fragmentStorageBufferBindings[MAX_STORAGE_BUFFERS_PER_STAGE];

    VkImageView computeSamplerTextureViewBindings[MAX_TEXTURE_SAMPLERS_PER_STAGE];
    VkSampler computeSamplerBindings[MAX_TEXTURE_SAMPLERS_PER_STAGE];
    VkImageView readOnlyComputeStorageTextureViewBindings[MAX_STORAGE_TEXTURES_PER_STAGE];
    VkBuffer readOnlyComputeStorageBufferBindings[MAX_STORAGE_BUFFERS_PER_STAGE];

    // Track these separately because barriers can happen mid compute pass
    VulkanTexture* readOnlyComputeStorageTextures[MAX_STORAGE_TEXTURES_PER_STAGE];
    VulkanBuffer* readOnlyComputeStorageBuffers[MAX_STORAGE_BUFFERS_PER_STAGE];

    VkImageView readWriteComputeStorageTextureViewBindings[MAX_COMPUTE_WRITE_TEXTURES];
    VkBuffer readWriteComputeStorageBufferBindings[MAX_COMPUTE_WRITE_BUFFERS];

    // Track these separately because they are barriered when the compute pass begins
    VulkanTextureSubresource* readWriteComputeStorageTextureSubresources[MAX_COMPUTE_WRITE_TEXTURES];
    Uint32 readWriteComputeStorageTextureSubresourceCount;
    VulkanBuffer* readWriteComputeStorageBuffers[MAX_COMPUTE_WRITE_BUFFERS];

    // Uniform buffers

    VulkanUniformBuffer* vertexUniformBuffers[MAX_UNIFORM_BUFFERS_PER_STAGE];
    VulkanUniformBuffer* fragmentUniformBuffers[MAX_UNIFORM_BUFFERS_PER_STAGE];
    VulkanUniformBuffer* computeUniformBuffers[MAX_UNIFORM_BUFFERS_PER_STAGE];

    // Track used resources

    VulkanBuffer** usedBuffers;
    Sint32 usedBufferCount;
    Sint32 usedBufferCapacity;

    VulkanTexture** usedTextures;
    Sint32 usedTextureCount;
    Sint32 usedTextureCapacity;

    VulkanSampler** usedSamplers;
    Sint32 usedSamplerCount;
    Sint32 usedSamplerCapacity;

    VulkanGraphicsPipeline** usedGraphicsPipelines;
    Sint32 usedGraphicsPipelineCount;
    Sint32 usedGraphicsPipelineCapacity;

    VulkanComputePipeline** usedComputePipelines;
    Sint32 usedComputePipelineCount;
    Sint32 usedComputePipelineCapacity;

    VulkanFramebuffer** usedFramebuffers;
    Sint32 usedFramebufferCount;
    Sint32 usedFramebufferCapacity;

    VulkanUniformBuffer** usedUniformBuffers;
    Sint32 usedUniformBufferCount;
    Sint32 usedUniformBufferCapacity;

    VulkanFenceHandle* inFlightFence;
    bool autoReleaseFence;

    bool swapchainRequested;
    bool isDefrag; // Whether this CB was created for defragging
} VulkanCommandBuffer;

struct VulkanCommandPool
{
    SDL_ThreadID threadID;
    VkCommandPool commandPool;

    VulkanCommandBuffer** inactiveCommandBuffers;
    Uint32 inactiveCommandBufferCapacity;
    Uint32 inactiveCommandBufferCount;
};

// Feature Checks

typedef struct VulkanFeatures
{
    Uint32 desiredApiVersion;
    VkPhysicalDeviceFeatures desiredVulkan10DeviceFeatures;
    VkPhysicalDeviceVulkan11Features desiredVulkan11DeviceFeatures;
    VkPhysicalDeviceVulkan12Features desiredVulkan12DeviceFeatures;
    VkPhysicalDeviceVulkan13Features desiredVulkan13DeviceFeatures;

    bool usesCustomVulkanOptions;

    Uint32 additionalDeviceExtensionCount;
    const char** additionalDeviceExtensionNames;
    Uint32 additionalInstanceExtensionCount;
    const char** additionalInstanceExtensionNames;
} VulkanFeatures;

// Context

struct VulkanRenderer
{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties2KHR physicalDeviceProperties;
    VkPhysicalDeviceDriverPropertiesKHR physicalDeviceDriverProperties;
    VkDevice logicalDevice;
    Uint8 integratedMemoryNotification;
    Uint8 outOfDeviceLocalMemoryWarning;
    Uint8 outofBARMemoryWarning;
    Uint8 fillModeOnlyWarning;

    bool debugMode;
    bool preferLowPower;
    bool requireHardwareAcceleration;
    SDL_PropertiesID props;
    Uint32 allowedFramesInFlight;

    VulkanExtensions supports;
    bool supportsDebugUtils;
    bool supportsColorspace;
    bool supportsPhysicalDeviceProperties2;
    bool supportsFillModeNonSolid;
    bool supportsMultiDrawIndirect;

    VulkanMemoryAllocator* memoryAllocator;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    bool checkEmptyAllocations;

    WindowData** claimedWindows;
    Uint32 claimedWindowCount;
    Uint32 claimedWindowCapacity;

    Uint32 queueFamilyIndex;
    VkQueue unifiedQueue;

    VulkanCommandBuffer** submittedCommandBuffers;
    Uint32 submittedCommandBufferCount;
    Uint32 submittedCommandBufferCapacity;

    VulkanFencePool fencePool;

    SDL_HashTable* commandPoolHashTable;
    SDL_HashTable* renderPassHashTable;
    SDL_HashTable* framebufferHashTable;
    SDL_HashTable* graphicsPipelineResourceLayoutHashTable;
    SDL_HashTable* computePipelineResourceLayoutHashTable;
    SDL_HashTable* descriptorSetLayoutHashTable;

    VulkanUniformBuffer** uniformBufferPool;
    Uint32 uniformBufferPoolCount;
    Uint32 uniformBufferPoolCapacity;

    DescriptorSetCache** descriptorSetCachePool;
    Uint32 descriptorSetCachePoolCount;
    Uint32 descriptorSetCachePoolCapacity;

    SDL_AtomicInt layoutResourceID;

    Uint32 minUBOAlignment;

    // Deferred resource destruction

    VulkanTexture** texturesToDestroy;
    Uint32 texturesToDestroyCount;
    Uint32 texturesToDestroyCapacity;

    VulkanBuffer** buffersToDestroy;
    Uint32 buffersToDestroyCount;
    Uint32 buffersToDestroyCapacity;

    VulkanSampler** samplersToDestroy;
    Uint32 samplersToDestroyCount;
    Uint32 samplersToDestroyCapacity;

    VulkanGraphicsPipeline** graphicsPipelinesToDestroy;
    Uint32 graphicsPipelinesToDestroyCount;
    Uint32 graphicsPipelinesToDestroyCapacity;

    VulkanComputePipeline** computePipelinesToDestroy;
    Uint32 computePipelinesToDestroyCount;
    Uint32 computePipelinesToDestroyCapacity;

    VulkanShader** shadersToDestroy;
    Uint32 shadersToDestroyCount;
    Uint32 shadersToDestroyCapacity;

    VulkanFramebuffer** framebuffersToDestroy;
    Uint32 framebuffersToDestroyCount;
    Uint32 framebuffersToDestroyCapacity;

    SDL_Mutex* allocatorLock;
    SDL_Mutex* disposeLock;
    SDL_Mutex* submitLock;
    SDL_Mutex* acquireCommandBufferLock;
    SDL_Mutex* acquireUniformBufferLock;
    SDL_Mutex* renderPassFetchLock;
    SDL_Mutex* framebufferFetchLock;
    SDL_Mutex* graphicsPipelineLayoutFetchLock;
    SDL_Mutex* computePipelineLayoutFetchLock;
    SDL_Mutex* descriptorSetLayoutFetchLock;
    SDL_Mutex* windowLock;

    Uint8 defragInProgress;

    VulkanMemoryAllocation** allocationsToDefrag;
    Uint32 allocationsToDefragCount;
    Uint32 allocationsToDefragCapacity;

#define VULKAN_INSTANCE_FUNCTION(func) \
    PFN_##func func;
#define VULKAN_DEVICE_FUNCTION(func) \
    PFN_##func func;
#include "gpu/vulkan/SDL_gpu_vulkan_vkfuncs.h"
};


#undef MAX_TEXTURE_SAMPLERS_PER_STAGE
#undef MAX_STORAGE_TEXTURES_PER_STAGE
#undef MAX_STORAGE_BUFFERS_PER_STAGE 
#undef MAX_UNIFORM_BUFFERS_PER_STAGE 
#undef MAX_COMPUTE_WRITE_TEXTURES    
#undef MAX_COMPUTE_WRITE_BUFFERS     
#undef UNIFORM_BUFFER_SIZE           
#undef MAX_VERTEX_BUFFERS            
#undef MAX_VERTEX_ATTRIBUTES         
#undef MAX_COLOR_TARGET_BINDINGS     
#undef MAX_PRESENT_COUNT             
#undef MAX_FRAMES_IN_FLIGHT 

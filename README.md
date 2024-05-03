# vk_wrapper_cpp
Compact Vulkan API wrapper in C++, based of https://vulkan-tutorial.com/ with chapter 27 depth buffering. Wrapper is written with simple class system and supports rotating camera and resolution scaling.

### ToDo
- [x] fix swapchain image formats
  - it is fixed by just closing MSI Afterburner
- [x] add precompiler for disabling features
- [x] add resolution scaling
  - it works, but it is better to just downscale your display resolution
  - ok now you can just use fullscreen feature
- [x] add camera rotation
  - it works, but it is better to just rotate your display
- [x] basic movement
- [x] add screen quad vertices
- [x] fix winding order of cube vertices
- [x] maybe at Dear ImGui in the future (maybe)
- [x] add fullscreen feature with F11
- [ ] fix winding order of cube vertices (so that back culling works)
- [ ] add compute functionality (maybe)
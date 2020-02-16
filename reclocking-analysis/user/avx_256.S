.code64

.section .avx_dp_fma_256, "ax"
vfmaddsub132pd %ymm0, %ymm1, %ymm2

.section .avx_dp_fma_256_unrolled, "ax"
vfmaddsub132pd %ymm0, %ymm0, %ymm1
vfmaddsub132pd %ymm0, %ymm0, %ymm2
vfmaddsub132pd %ymm0, %ymm0, %ymm3
vfmaddsub132pd %ymm0, %ymm0, %ymm4
vfmaddsub132pd %ymm0, %ymm0, %ymm5
vfmaddsub132pd %ymm0, %ymm0, %ymm6
vfmaddsub132pd %ymm0, %ymm0, %ymm7
vfmaddsub132pd %ymm0, %ymm0, %ymm8
vfmaddsub132pd %ymm0, %ymm0, %ymm9
vfmaddsub132pd %ymm0, %ymm0, %ymm10
vfmaddsub132pd %ymm0, %ymm0, %ymm11
vfmaddsub132pd %ymm0, %ymm0, %ymm12
vfmaddsub132pd %ymm0, %ymm0, %ymm13
vfmaddsub132pd %ymm0, %ymm0, %ymm14
vfmaddsub132pd %ymm0, %ymm0, %ymm15
vfmaddsub132pd %ymm0, %ymm0, %ymm16
vfmaddsub132pd %ymm0, %ymm0, %ymm17
vfmaddsub132pd %ymm0, %ymm0, %ymm18
vfmaddsub132pd %ymm0, %ymm0, %ymm19
vfmaddsub132pd %ymm0, %ymm0, %ymm20
vfmaddsub132pd %ymm0, %ymm0, %ymm21
vfmaddsub132pd %ymm0, %ymm0, %ymm22
vfmaddsub132pd %ymm0, %ymm0, %ymm23
vfmaddsub132pd %ymm0, %ymm0, %ymm24
vfmaddsub132pd %ymm0, %ymm0, %ymm25
vfmaddsub132pd %ymm0, %ymm0, %ymm26
vfmaddsub132pd %ymm0, %ymm0, %ymm27
vfmaddsub132pd %ymm0, %ymm0, %ymm28
vfmaddsub132pd %ymm0, %ymm0, %ymm29
vfmaddsub132pd %ymm0, %ymm0, %ymm30
vfmaddsub132pd %ymm0, %ymm0, %ymm31

.section .avx_sp_fma_256, "ax"
vfmaddsub132ps %ymm0, %ymm1, %ymm2

.section .avx_sp_fma_256_unrolled, "ax"
vfmaddsub132ps %ymm0, %ymm0, %ymm1
vfmaddsub132ps %ymm0, %ymm0, %ymm2
vfmaddsub132ps %ymm0, %ymm0, %ymm3
vfmaddsub132ps %ymm0, %ymm0, %ymm4
vfmaddsub132ps %ymm0, %ymm0, %ymm5
vfmaddsub132ps %ymm0, %ymm0, %ymm6
vfmaddsub132ps %ymm0, %ymm0, %ymm7
vfmaddsub132ps %ymm0, %ymm0, %ymm8
vfmaddsub132ps %ymm0, %ymm0, %ymm9
vfmaddsub132ps %ymm0, %ymm0, %ymm10
vfmaddsub132ps %ymm0, %ymm0, %ymm11
vfmaddsub132ps %ymm0, %ymm0, %ymm12
vfmaddsub132ps %ymm0, %ymm0, %ymm13
vfmaddsub132ps %ymm0, %ymm0, %ymm14
vfmaddsub132ps %ymm0, %ymm0, %ymm15
vfmaddsub132ps %ymm0, %ymm0, %ymm16
vfmaddsub132ps %ymm0, %ymm0, %ymm17
vfmaddsub132ps %ymm0, %ymm0, %ymm18
vfmaddsub132ps %ymm0, %ymm0, %ymm19
vfmaddsub132ps %ymm0, %ymm0, %ymm20
vfmaddsub132ps %ymm0, %ymm0, %ymm21
vfmaddsub132ps %ymm0, %ymm0, %ymm22
vfmaddsub132ps %ymm0, %ymm0, %ymm23
vfmaddsub132ps %ymm0, %ymm0, %ymm24
vfmaddsub132ps %ymm0, %ymm0, %ymm25
vfmaddsub132ps %ymm0, %ymm0, %ymm26
vfmaddsub132ps %ymm0, %ymm0, %ymm27
vfmaddsub132ps %ymm0, %ymm0, %ymm28
vfmaddsub132ps %ymm0, %ymm0, %ymm29
vfmaddsub132ps %ymm0, %ymm0, %ymm30
vfmaddsub132ps %ymm0, %ymm0, %ymm31

.section .avx_dp_mul_256, "ax"
vmulpd %ymm0, %ymm1, %ymm2

.section .avx_dp_mul_256_unrolled, "ax"
vmulpd %ymm0, %ymm0, %ymm1
vmulpd %ymm0, %ymm0, %ymm2
vmulpd %ymm0, %ymm0, %ymm3
vmulpd %ymm0, %ymm0, %ymm4
vmulpd %ymm0, %ymm0, %ymm5
vmulpd %ymm0, %ymm0, %ymm6
vmulpd %ymm0, %ymm0, %ymm7
vmulpd %ymm0, %ymm0, %ymm8
vmulpd %ymm0, %ymm0, %ymm9
vmulpd %ymm0, %ymm0, %ymm10
vmulpd %ymm0, %ymm0, %ymm11
vmulpd %ymm0, %ymm0, %ymm12
vmulpd %ymm0, %ymm0, %ymm13
vmulpd %ymm0, %ymm0, %ymm14
vmulpd %ymm0, %ymm0, %ymm15
vmulpd %ymm0, %ymm0, %ymm16
vmulpd %ymm0, %ymm0, %ymm17
vmulpd %ymm0, %ymm0, %ymm18
vmulpd %ymm0, %ymm0, %ymm19
vmulpd %ymm0, %ymm0, %ymm20
vmulpd %ymm0, %ymm0, %ymm21
vmulpd %ymm0, %ymm0, %ymm22
vmulpd %ymm0, %ymm0, %ymm23
vmulpd %ymm0, %ymm0, %ymm24
vmulpd %ymm0, %ymm0, %ymm25
vmulpd %ymm0, %ymm0, %ymm26
vmulpd %ymm0, %ymm0, %ymm27
vmulpd %ymm0, %ymm0, %ymm28
vmulpd %ymm0, %ymm0, %ymm29
vmulpd %ymm0, %ymm0, %ymm30
vmulpd %ymm0, %ymm0, %ymm31

.section .avx_sp_mul_256, "ax"
vmulps %ymm0, %ymm1, %ymm2

.section .avx_sp_mul_256_unrolled, "ax"
vmulps %ymm0, %ymm0, %ymm1
vmulps %ymm0, %ymm0, %ymm2
vmulps %ymm0, %ymm0, %ymm3
vmulps %ymm0, %ymm0, %ymm4
vmulps %ymm0, %ymm0, %ymm5
vmulps %ymm0, %ymm0, %ymm6
vmulps %ymm0, %ymm0, %ymm7
vmulps %ymm0, %ymm0, %ymm8
vmulps %ymm0, %ymm0, %ymm9
vmulps %ymm0, %ymm0, %ymm10
vmulps %ymm0, %ymm0, %ymm11
vmulps %ymm0, %ymm0, %ymm12
vmulps %ymm0, %ymm0, %ymm13
vmulps %ymm0, %ymm0, %ymm14
vmulps %ymm0, %ymm0, %ymm15
vmulps %ymm0, %ymm0, %ymm16
vmulps %ymm0, %ymm0, %ymm17
vmulps %ymm0, %ymm0, %ymm18
vmulps %ymm0, %ymm0, %ymm19
vmulps %ymm0, %ymm0, %ymm20
vmulps %ymm0, %ymm0, %ymm21
vmulps %ymm0, %ymm0, %ymm22
vmulps %ymm0, %ymm0, %ymm23
vmulps %ymm0, %ymm0, %ymm24
vmulps %ymm0, %ymm0, %ymm25
vmulps %ymm0, %ymm0, %ymm26
vmulps %ymm0, %ymm0, %ymm27
vmulps %ymm0, %ymm0, %ymm28
vmulps %ymm0, %ymm0, %ymm29
vmulps %ymm0, %ymm0, %ymm30
vmulps %ymm0, %ymm0, %ymm31

.section .avx_int_mul_256, "ax"
vpmullq %ymm0, %ymm1, %ymm2

.section .avx_int_mul_256_unrolled, "ax"
vpmullq %ymm0, %ymm0, %ymm1
vpmullq %ymm0, %ymm0, %ymm2
vpmullq %ymm0, %ymm0, %ymm3
vpmullq %ymm0, %ymm0, %ymm4
vpmullq %ymm0, %ymm0, %ymm5
vpmullq %ymm0, %ymm0, %ymm6
vpmullq %ymm0, %ymm0, %ymm7
vpmullq %ymm0, %ymm0, %ymm8
vpmullq %ymm0, %ymm0, %ymm9
vpmullq %ymm0, %ymm0, %ymm10
vpmullq %ymm0, %ymm0, %ymm11
vpmullq %ymm0, %ymm0, %ymm12
vpmullq %ymm0, %ymm0, %ymm13
vpmullq %ymm0, %ymm0, %ymm14
vpmullq %ymm0, %ymm0, %ymm15
vpmullq %ymm0, %ymm0, %ymm16
vpmullq %ymm0, %ymm0, %ymm17
vpmullq %ymm0, %ymm0, %ymm18
vpmullq %ymm0, %ymm0, %ymm19
vpmullq %ymm0, %ymm0, %ymm20
vpmullq %ymm0, %ymm0, %ymm21
vpmullq %ymm0, %ymm0, %ymm22
vpmullq %ymm0, %ymm0, %ymm23
vpmullq %ymm0, %ymm0, %ymm24
vpmullq %ymm0, %ymm0, %ymm25
vpmullq %ymm0, %ymm0, %ymm26
vpmullq %ymm0, %ymm0, %ymm27
vpmullq %ymm0, %ymm0, %ymm28
vpmullq %ymm0, %ymm0, %ymm29
vpmullq %ymm0, %ymm0, %ymm30
vpmullq %ymm0, %ymm0, %ymm31

.section .avx_int_dw_to_w_256, "ax"
vpackssdw %ymm0, %ymm1, %ymm2

.section .avx_int_dw_to_w_256_unrolled, "ax"
vpackssdw %ymm0, %ymm0, %ymm1
vpackssdw %ymm0, %ymm0, %ymm2
vpackssdw %ymm0, %ymm0, %ymm3
vpackssdw %ymm0, %ymm0, %ymm4
vpackssdw %ymm0, %ymm0, %ymm5
vpackssdw %ymm0, %ymm0, %ymm6
vpackssdw %ymm0, %ymm0, %ymm7
vpackssdw %ymm0, %ymm0, %ymm8
vpackssdw %ymm0, %ymm0, %ymm9
vpackssdw %ymm0, %ymm0, %ymm10
vpackssdw %ymm0, %ymm0, %ymm11
vpackssdw %ymm0, %ymm0, %ymm12
vpackssdw %ymm0, %ymm0, %ymm13
vpackssdw %ymm0, %ymm0, %ymm14
vpackssdw %ymm0, %ymm0, %ymm15
vpackssdw %ymm0, %ymm0, %ymm16
vpackssdw %ymm0, %ymm0, %ymm17
vpackssdw %ymm0, %ymm0, %ymm18
vpackssdw %ymm0, %ymm0, %ymm19
vpackssdw %ymm0, %ymm0, %ymm20
vpackssdw %ymm0, %ymm0, %ymm21
vpackssdw %ymm0, %ymm0, %ymm22
vpackssdw %ymm0, %ymm0, %ymm23
vpackssdw %ymm0, %ymm0, %ymm24
vpackssdw %ymm0, %ymm0, %ymm25
vpackssdw %ymm0, %ymm0, %ymm26
vpackssdw %ymm0, %ymm0, %ymm27
vpackssdw %ymm0, %ymm0, %ymm28
vpackssdw %ymm0, %ymm0, %ymm29
vpackssdw %ymm0, %ymm0, %ymm30
vpackssdw %ymm0, %ymm0, %ymm31

.section .avx_int_add_256, "ax"
vpaddsw %ymm0, %ymm1, %ymm2

.section .avx_int_add_256_unrolled, "ax"
vpaddsw %ymm0, %ymm0, %ymm1
vpaddsw %ymm0, %ymm0, %ymm2
vpaddsw %ymm0, %ymm0, %ymm3
vpaddsw %ymm0, %ymm0, %ymm4
vpaddsw %ymm0, %ymm0, %ymm5
vpaddsw %ymm0, %ymm0, %ymm6
vpaddsw %ymm0, %ymm0, %ymm7
vpaddsw %ymm0, %ymm0, %ymm8
vpaddsw %ymm0, %ymm0, %ymm9
vpaddsw %ymm0, %ymm0, %ymm10
vpaddsw %ymm0, %ymm0, %ymm11
vpaddsw %ymm0, %ymm0, %ymm12
vpaddsw %ymm0, %ymm0, %ymm13
vpaddsw %ymm0, %ymm0, %ymm14
vpaddsw %ymm0, %ymm0, %ymm15
vpaddsw %ymm0, %ymm0, %ymm16
vpaddsw %ymm0, %ymm0, %ymm17
vpaddsw %ymm0, %ymm0, %ymm18
vpaddsw %ymm0, %ymm0, %ymm19
vpaddsw %ymm0, %ymm0, %ymm20
vpaddsw %ymm0, %ymm0, %ymm21
vpaddsw %ymm0, %ymm0, %ymm22
vpaddsw %ymm0, %ymm0, %ymm23
vpaddsw %ymm0, %ymm0, %ymm24
vpaddsw %ymm0, %ymm0, %ymm25
vpaddsw %ymm0, %ymm0, %ymm26
vpaddsw %ymm0, %ymm0, %ymm27
vpaddsw %ymm0, %ymm0, %ymm28
vpaddsw %ymm0, %ymm0, %ymm29
vpaddsw %ymm0, %ymm0, %ymm30
vpaddsw %ymm0, %ymm0, %ymm31

.section .avx_int_mul_add_256, "ax"
vpmaddwd %ymm0, %ymm1, %ymm2

.section .avx_int_mul_add_256_unrolled, "ax"
vpmaddwd %ymm0, %ymm0, %ymm1
vpmaddwd %ymm0, %ymm0, %ymm2
vpmaddwd %ymm0, %ymm0, %ymm3
vpmaddwd %ymm0, %ymm0, %ymm4
vpmaddwd %ymm0, %ymm0, %ymm5
vpmaddwd %ymm0, %ymm0, %ymm6
vpmaddwd %ymm0, %ymm0, %ymm7
vpmaddwd %ymm0, %ymm0, %ymm8
vpmaddwd %ymm0, %ymm0, %ymm9
vpmaddwd %ymm0, %ymm0, %ymm10
vpmaddwd %ymm0, %ymm0, %ymm11
vpmaddwd %ymm0, %ymm0, %ymm12
vpmaddwd %ymm0, %ymm0, %ymm13
vpmaddwd %ymm0, %ymm0, %ymm14
vpmaddwd %ymm0, %ymm0, %ymm15
vpmaddwd %ymm0, %ymm0, %ymm16
vpmaddwd %ymm0, %ymm0, %ymm17
vpmaddwd %ymm0, %ymm0, %ymm18
vpmaddwd %ymm0, %ymm0, %ymm19
vpmaddwd %ymm0, %ymm0, %ymm20
vpmaddwd %ymm0, %ymm0, %ymm21
vpmaddwd %ymm0, %ymm0, %ymm22
vpmaddwd %ymm0, %ymm0, %ymm23
vpmaddwd %ymm0, %ymm0, %ymm24
vpmaddwd %ymm0, %ymm0, %ymm25
vpmaddwd %ymm0, %ymm0, %ymm26
vpmaddwd %ymm0, %ymm0, %ymm27
vpmaddwd %ymm0, %ymm0, %ymm28
vpmaddwd %ymm0, %ymm0, %ymm29
vpmaddwd %ymm0, %ymm0, %ymm30
vpmaddwd %ymm0, %ymm0, %ymm31

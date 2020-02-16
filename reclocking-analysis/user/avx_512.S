.code64

// Fused Multiply-Alternating Add/Subtract of Packed Double-Precision Floating-Point Values
.section .avx_dp_fma_512, "ax"
vfmaddsub132pd %zmm0, %zmm1, %zmm2

.section .avx_dp_fma_512_unrolled, "ax"
vfmaddsub132pd %zmm0, %zmm0, %zmm1
vfmaddsub132pd %zmm0, %zmm0, %zmm2
vfmaddsub132pd %zmm0, %zmm0, %zmm3
vfmaddsub132pd %zmm0, %zmm0, %zmm4
vfmaddsub132pd %zmm0, %zmm0, %zmm5
vfmaddsub132pd %zmm0, %zmm0, %zmm6
vfmaddsub132pd %zmm0, %zmm0, %zmm7
vfmaddsub132pd %zmm0, %zmm0, %zmm8
vfmaddsub132pd %zmm0, %zmm0, %zmm9
vfmaddsub132pd %zmm0, %zmm0, %zmm10
vfmaddsub132pd %zmm0, %zmm0, %zmm11
vfmaddsub132pd %zmm0, %zmm0, %zmm12
vfmaddsub132pd %zmm0, %zmm0, %zmm13
vfmaddsub132pd %zmm0, %zmm0, %zmm14
vfmaddsub132pd %zmm0, %zmm0, %zmm15
vfmaddsub132pd %zmm0, %zmm0, %zmm16
vfmaddsub132pd %zmm0, %zmm0, %zmm17
vfmaddsub132pd %zmm0, %zmm0, %zmm18
vfmaddsub132pd %zmm0, %zmm0, %zmm19
vfmaddsub132pd %zmm0, %zmm0, %zmm20
vfmaddsub132pd %zmm0, %zmm0, %zmm21
vfmaddsub132pd %zmm0, %zmm0, %zmm22
vfmaddsub132pd %zmm0, %zmm0, %zmm23
vfmaddsub132pd %zmm0, %zmm0, %zmm24
vfmaddsub132pd %zmm0, %zmm0, %zmm25
vfmaddsub132pd %zmm0, %zmm0, %zmm26
vfmaddsub132pd %zmm0, %zmm0, %zmm27
vfmaddsub132pd %zmm0, %zmm0, %zmm28
vfmaddsub132pd %zmm0, %zmm0, %zmm29
vfmaddsub132pd %zmm0, %zmm0, %zmm30
vfmaddsub132pd %zmm0, %zmm0, %zmm31

// Fused Multiply-Alternating Add/Subtract of Packed Single-Precision Floating-Point Values
.section .avx_sp_fma_512, "ax"
vfmaddsub132ps %zmm0, %zmm1, %zmm2

.section .avx_sp_fma_512_unrolled, "ax"
vfmaddsub132ps %zmm0, %zmm0, %zmm1
vfmaddsub132ps %zmm0, %zmm0, %zmm2
vfmaddsub132ps %zmm0, %zmm0, %zmm3
vfmaddsub132ps %zmm0, %zmm0, %zmm4
vfmaddsub132ps %zmm0, %zmm0, %zmm5
vfmaddsub132ps %zmm0, %zmm0, %zmm6
vfmaddsub132ps %zmm0, %zmm0, %zmm7
vfmaddsub132ps %zmm0, %zmm0, %zmm8
vfmaddsub132ps %zmm0, %zmm0, %zmm9
vfmaddsub132ps %zmm0, %zmm0, %zmm10
vfmaddsub132ps %zmm0, %zmm0, %zmm11
vfmaddsub132ps %zmm0, %zmm0, %zmm12
vfmaddsub132ps %zmm0, %zmm0, %zmm13
vfmaddsub132ps %zmm0, %zmm0, %zmm14
vfmaddsub132ps %zmm0, %zmm0, %zmm15
vfmaddsub132ps %zmm0, %zmm0, %zmm16
vfmaddsub132ps %zmm0, %zmm0, %zmm17
vfmaddsub132ps %zmm0, %zmm0, %zmm18
vfmaddsub132ps %zmm0, %zmm0, %zmm19
vfmaddsub132ps %zmm0, %zmm0, %zmm20
vfmaddsub132ps %zmm0, %zmm0, %zmm21
vfmaddsub132ps %zmm0, %zmm0, %zmm22
vfmaddsub132ps %zmm0, %zmm0, %zmm23
vfmaddsub132ps %zmm0, %zmm0, %zmm24
vfmaddsub132ps %zmm0, %zmm0, %zmm25
vfmaddsub132ps %zmm0, %zmm0, %zmm26
vfmaddsub132ps %zmm0, %zmm0, %zmm27
vfmaddsub132ps %zmm0, %zmm0, %zmm28
vfmaddsub132ps %zmm0, %zmm0, %zmm29
vfmaddsub132ps %zmm0, %zmm0, %zmm30
vfmaddsub132ps %zmm0, %zmm0, %zmm31

// Multiply Packed Double-Precision Floating-Point Values
.section .avx_dp_mul_512, "ax"
vmulpd %zmm0, %zmm1, %zmm2

.section .avx_dp_mul_512_unrolled, "ax"
vmulpd %zmm0, %zmm0, %zmm1
vmulpd %zmm0, %zmm0, %zmm2
vmulpd %zmm0, %zmm0, %zmm3
vmulpd %zmm0, %zmm0, %zmm4
vmulpd %zmm0, %zmm0, %zmm5
vmulpd %zmm0, %zmm0, %zmm6
vmulpd %zmm0, %zmm0, %zmm7
vmulpd %zmm0, %zmm0, %zmm8
vmulpd %zmm0, %zmm0, %zmm9
vmulpd %zmm0, %zmm0, %zmm10
vmulpd %zmm0, %zmm0, %zmm11
vmulpd %zmm0, %zmm0, %zmm12
vmulpd %zmm0, %zmm0, %zmm13
vmulpd %zmm0, %zmm0, %zmm14
vmulpd %zmm0, %zmm0, %zmm15
vmulpd %zmm0, %zmm0, %zmm16
vmulpd %zmm0, %zmm0, %zmm17
vmulpd %zmm0, %zmm0, %zmm18
vmulpd %zmm0, %zmm0, %zmm19
vmulpd %zmm0, %zmm0, %zmm20
vmulpd %zmm0, %zmm0, %zmm21
vmulpd %zmm0, %zmm0, %zmm22
vmulpd %zmm0, %zmm0, %zmm23
vmulpd %zmm0, %zmm0, %zmm24
vmulpd %zmm0, %zmm0, %zmm25
vmulpd %zmm0, %zmm0, %zmm26
vmulpd %zmm0, %zmm0, %zmm27
vmulpd %zmm0, %zmm0, %zmm28
vmulpd %zmm0, %zmm0, %zmm29
vmulpd %zmm0, %zmm0, %zmm30
vmulpd %zmm0, %zmm0, %zmm31

// Multiply Packed Single-Precision Floating-Point Values
.section .avx_sp_mul_512, "ax"
vmulps %zmm0, %zmm1, %zmm2

.section .avx_sp_mul_512_unrolled, "ax"
vmulps %zmm0, %zmm0, %zmm1
vmulps %zmm0, %zmm0, %zmm2
vmulps %zmm0, %zmm0, %zmm3
vmulps %zmm0, %zmm0, %zmm4
vmulps %zmm0, %zmm0, %zmm5
vmulps %zmm0, %zmm0, %zmm6
vmulps %zmm0, %zmm0, %zmm7
vmulps %zmm0, %zmm0, %zmm8
vmulps %zmm0, %zmm0, %zmm9
vmulps %zmm0, %zmm0, %zmm10
vmulps %zmm0, %zmm0, %zmm11
vmulps %zmm0, %zmm0, %zmm12
vmulps %zmm0, %zmm0, %zmm13
vmulps %zmm0, %zmm0, %zmm14
vmulps %zmm0, %zmm0, %zmm15
vmulps %zmm0, %zmm0, %zmm16
vmulps %zmm0, %zmm0, %zmm17
vmulps %zmm0, %zmm0, %zmm18
vmulps %zmm0, %zmm0, %zmm19
vmulps %zmm0, %zmm0, %zmm20
vmulps %zmm0, %zmm0, %zmm21
vmulps %zmm0, %zmm0, %zmm22
vmulps %zmm0, %zmm0, %zmm23
vmulps %zmm0, %zmm0, %zmm24
vmulps %zmm0, %zmm0, %zmm25
vmulps %zmm0, %zmm0, %zmm26
vmulps %zmm0, %zmm0, %zmm27
vmulps %zmm0, %zmm0, %zmm28
vmulps %zmm0, %zmm0, %zmm29
vmulps %zmm0, %zmm0, %zmm30
vmulps %zmm0, %zmm0, %zmm31

// Multiply Packed Integers and Store Low Result
.section .avx_int_mul_512, "ax"
vpmullq %zmm0, %zmm1, %zmm2

.section .avx_int_mul_512_unrolled, "ax"
vpmullq %zmm0, %zmm0, %zmm1
vpmullq %zmm0, %zmm0, %zmm2
vpmullq %zmm0, %zmm0, %zmm3
vpmullq %zmm0, %zmm0, %zmm4
vpmullq %zmm0, %zmm0, %zmm5
vpmullq %zmm0, %zmm0, %zmm6
vpmullq %zmm0, %zmm0, %zmm7
vpmullq %zmm0, %zmm0, %zmm8
vpmullq %zmm0, %zmm0, %zmm9
vpmullq %zmm0, %zmm0, %zmm10
vpmullq %zmm0, %zmm0, %zmm11
vpmullq %zmm0, %zmm0, %zmm12
vpmullq %zmm0, %zmm0, %zmm13
vpmullq %zmm0, %zmm0, %zmm14
vpmullq %zmm0, %zmm0, %zmm15
vpmullq %zmm0, %zmm0, %zmm16
vpmullq %zmm0, %zmm0, %zmm17
vpmullq %zmm0, %zmm0, %zmm18
vpmullq %zmm0, %zmm0, %zmm19
vpmullq %zmm0, %zmm0, %zmm20
vpmullq %zmm0, %zmm0, %zmm21
vpmullq %zmm0, %zmm0, %zmm22
vpmullq %zmm0, %zmm0, %zmm23
vpmullq %zmm0, %zmm0, %zmm24
vpmullq %zmm0, %zmm0, %zmm25
vpmullq %zmm0, %zmm0, %zmm26
vpmullq %zmm0, %zmm0, %zmm27
vpmullq %zmm0, %zmm0, %zmm28
vpmullq %zmm0, %zmm0, %zmm29
vpmullq %zmm0, %zmm0, %zmm30
vpmullq %zmm0, %zmm0, %zmm31

// Pack with Signed Saturation
.section .avx_int_dw_to_w_512, "ax"
vpackssdw %zmm0, %zmm1, %zmm2

.section .avx_int_dw_to_w_512_unrolled, "ax"
vpackssdw %zmm0, %zmm0, %zmm1
vpackssdw %zmm0, %zmm0, %zmm2
vpackssdw %zmm0, %zmm0, %zmm3
vpackssdw %zmm0, %zmm0, %zmm4
vpackssdw %zmm0, %zmm0, %zmm5
vpackssdw %zmm0, %zmm0, %zmm6
vpackssdw %zmm0, %zmm0, %zmm7
vpackssdw %zmm0, %zmm0, %zmm8
vpackssdw %zmm0, %zmm0, %zmm9
vpackssdw %zmm0, %zmm0, %zmm10
vpackssdw %zmm0, %zmm0, %zmm11
vpackssdw %zmm0, %zmm0, %zmm12
vpackssdw %zmm0, %zmm0, %zmm13
vpackssdw %zmm0, %zmm0, %zmm14
vpackssdw %zmm0, %zmm0, %zmm15
vpackssdw %zmm0, %zmm0, %zmm16
vpackssdw %zmm0, %zmm0, %zmm17
vpackssdw %zmm0, %zmm0, %zmm18
vpackssdw %zmm0, %zmm0, %zmm19
vpackssdw %zmm0, %zmm0, %zmm20
vpackssdw %zmm0, %zmm0, %zmm21
vpackssdw %zmm0, %zmm0, %zmm22
vpackssdw %zmm0, %zmm0, %zmm23
vpackssdw %zmm0, %zmm0, %zmm24
vpackssdw %zmm0, %zmm0, %zmm25
vpackssdw %zmm0, %zmm0, %zmm26
vpackssdw %zmm0, %zmm0, %zmm27
vpackssdw %zmm0, %zmm0, %zmm28
vpackssdw %zmm0, %zmm0, %zmm29
vpackssdw %zmm0, %zmm0, %zmm30
vpackssdw %zmm0, %zmm0, %zmm31

// Add Packed Signed Integers with Signed Saturation
.section .avx_int_add_512, "ax"
vpaddsw %zmm0, %zmm1, %zmm2

.section .avx_int_add_512_unrolled, "ax"
vpaddsw %zmm0, %zmm0, %zmm1
vpaddsw %zmm0, %zmm0, %zmm2
vpaddsw %zmm0, %zmm0, %zmm3
vpaddsw %zmm0, %zmm0, %zmm4
vpaddsw %zmm0, %zmm0, %zmm5
vpaddsw %zmm0, %zmm0, %zmm6
vpaddsw %zmm0, %zmm0, %zmm7
vpaddsw %zmm0, %zmm0, %zmm8
vpaddsw %zmm0, %zmm0, %zmm9
vpaddsw %zmm0, %zmm0, %zmm10
vpaddsw %zmm0, %zmm0, %zmm11
vpaddsw %zmm0, %zmm0, %zmm12
vpaddsw %zmm0, %zmm0, %zmm13
vpaddsw %zmm0, %zmm0, %zmm14
vpaddsw %zmm0, %zmm0, %zmm15
vpaddsw %zmm0, %zmm0, %zmm16
vpaddsw %zmm0, %zmm0, %zmm17
vpaddsw %zmm0, %zmm0, %zmm18
vpaddsw %zmm0, %zmm0, %zmm19
vpaddsw %zmm0, %zmm0, %zmm20
vpaddsw %zmm0, %zmm0, %zmm21
vpaddsw %zmm0, %zmm0, %zmm22
vpaddsw %zmm0, %zmm0, %zmm23
vpaddsw %zmm0, %zmm0, %zmm24
vpaddsw %zmm0, %zmm0, %zmm25
vpaddsw %zmm0, %zmm0, %zmm26
vpaddsw %zmm0, %zmm0, %zmm27
vpaddsw %zmm0, %zmm0, %zmm28
vpaddsw %zmm0, %zmm0, %zmm29
vpaddsw %zmm0, %zmm0, %zmm30
vpaddsw %zmm0, %zmm0, %zmm31

// Multiply and Add Packed Integers
.section .avx_int_mul_add_512, "ax"
vpmaddwd %zmm0, %zmm1, %zmm2

.section .avx_int_mul_add_512_unrolled, "ax"
vpmaddwd %zmm0, %zmm0, %zmm1
vpmaddwd %zmm0, %zmm0, %zmm2
vpmaddwd %zmm0, %zmm0, %zmm3
vpmaddwd %zmm0, %zmm0, %zmm4
vpmaddwd %zmm0, %zmm0, %zmm5
vpmaddwd %zmm0, %zmm0, %zmm6
vpmaddwd %zmm0, %zmm0, %zmm7
vpmaddwd %zmm0, %zmm0, %zmm8
vpmaddwd %zmm0, %zmm0, %zmm9
vpmaddwd %zmm0, %zmm0, %zmm10
vpmaddwd %zmm0, %zmm0, %zmm11
vpmaddwd %zmm0, %zmm0, %zmm12
vpmaddwd %zmm0, %zmm0, %zmm13
vpmaddwd %zmm0, %zmm0, %zmm14
vpmaddwd %zmm0, %zmm0, %zmm15
vpmaddwd %zmm0, %zmm0, %zmm16
vpmaddwd %zmm0, %zmm0, %zmm17
vpmaddwd %zmm0, %zmm0, %zmm18
vpmaddwd %zmm0, %zmm0, %zmm19
vpmaddwd %zmm0, %zmm0, %zmm20
vpmaddwd %zmm0, %zmm0, %zmm21
vpmaddwd %zmm0, %zmm0, %zmm22
vpmaddwd %zmm0, %zmm0, %zmm23
vpmaddwd %zmm0, %zmm0, %zmm24
vpmaddwd %zmm0, %zmm0, %zmm25
vpmaddwd %zmm0, %zmm0, %zmm26
vpmaddwd %zmm0, %zmm0, %zmm27
vpmaddwd %zmm0, %zmm0, %zmm28
vpmaddwd %zmm0, %zmm0, %zmm29
vpmaddwd %zmm0, %zmm0, %zmm30
vpmaddwd %zmm0, %zmm0, %zmm31

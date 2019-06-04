/* 
  This file was generated by KreMLin <https://github.com/FStarLang/kremlin>
  KreMLin invocation: /data/everest/everest/kremlin/krml -bundle Hacl.Spec.*,Spec.*[rename=Hacl_Spec] -bundle Hacl.Poly1305.Field32xN.Lemmas[rename=Hacl_Lemmas] -bundle Lib.*[rename=Hacl_Lib] -drop Lib.IntVector.Intrinsics -add-include "libintvector.h" -add-include "evercrypt_targetconfig.h" -drop EverCrypt.TargetConfig -bundle EverCrypt.BCrypt -bundle EverCrypt.OpenSSL -bundle MerkleTree.Spec,MerkleTree.Spec.*,MerkleTree.New.High,MerkleTree.New.High.* -bundle Vale.Stdcalls.*,Vale.Interop,Vale.Interop.*,Vale.Wrapper.X64.*[rename=Vale] -bundle Vale.Inline.X64.*[rename=Vale_Inline] -bundle FStar.Tactics.CanonCommMonoid,FStar.Tactics.CanonCommSemiring,FStar.Tactics.CanonCommSwaps[rename=Unused] -bundle Vale.*[rename=Unused2] -library Vale.Stdcalls.* -static-header Vale_Inline -library Vale.Inline.X64.Fadd_inline -library Vale.Inline.X64.Fmul_inline -library Vale.Inline.X64.Fswap_inline -library Vale.Inline.X64.Fsqr_inline -no-prefix Vale.Stdcalls.* -no-prefix Vale.Inline.X64.Fadd_inline -no-prefix Vale.Inline.X64.Fmul_inline -no-prefix Vale.Inline.X64.Fswap_inline -no-prefix Vale.Inline.X64.Fsqr_inline -no-prefix EverCrypt.Vale -add-include "curve25519-inline.h" -no-prefix MerkleTree.New.Low -no-prefix MerkleTree.New.Low.Serialization -fparentheses -fno-shadow -fcurly-braces -bundle WasmSupport -bundle Test,Test.*,Hacl.Test.* -bundle Hacl.Hash.MD5+Hacl.Hash.Core.MD5+Hacl.Hash.SHA1+Hacl.Hash.Core.SHA1+Hacl.Hash.SHA2+Hacl.Hash.Core.SHA2+Hacl.Hash.Core.SHA2.Constants=Hacl.Hash.*[rename=Hacl_Hash] -bundle Hacl.Impl.SHA3+Hacl.SHA3=[rename=Hacl_SHA3] -bundle Hacl.Impl.Poly1305.*[rename=Unused_Poly1305] -bundle Hacl.Impl.Chacha20=Hacl.Impl.Chacha20.*[rename=Hacl_Chacha20] -bundle Hacl.Curve25519_51+Hacl.Curve25519_64=Hacl.Impl.Curve25519.*[rename=Hacl_Curve25519] -bundle Hacl.Impl.Chacha20Poly1305=Hacl.Impl.Chacha20Poly1305.*[rename=Hacl_Chacha20Poly1305] -bundle Hacl.Ed25519=Hacl.Impl.Ed25519.*,Hacl.Impl.BignumQ.Mul,Hacl.Impl.Load56,Hacl.Impl.SHA512.ModQ,Hacl.Impl.Store56,Hacl.Bignum25519 -bundle LowStar.* -bundle Prims,C.Failure,C,C.String,C.Loops,Spec.Loops,C.Endianness,FStar.*[rename=Hacl_Kremlib] -bundle EverCrypt.Spec.* -bundle MerkleTree.New.Low+MerkleTree.New.Low.Serialization=[rename=MerkleTree] -bundle Test,Test.*,WindowsHack -bundle EverCrypt.Hash+EverCrypt.Hash.Incremental=[rename=EverCrypt_Hash] -library EverCrypt.AutoConfig,EverCrypt.OpenSSL,EverCrypt.BCrypt -minimal -add-include "kremlin/internal/types.h" -add-include "kremlin/internal/target.h" -add-include "kremlin/lowstar_endianness.h" -add-include <string.h> -fbuiltin-uint128 -bundle EverCrypt.AutoConfig2= -bundle EverCrypt -bundle EverCrypt.Hacl -bundle \*[rename=EverCrypt_Misc] -tmpdir dist/ccf/ -skip-compilation obj/prims.krml obj/FStar_Pervasives_Native.krml obj/FStar_Pervasives.krml obj/FStar_Float.krml obj/FStar_Mul.krml obj/FStar_Preorder.krml obj/FStar_Calc.krml obj/FStar_Squash.krml obj/FStar_Classical.krml obj/FStar_StrongExcludedMiddle.krml obj/FStar_FunctionalExtensionality.krml obj/FStar_List_Tot_Base.krml obj/FStar_List_Tot_Properties.krml obj/FStar_List_Tot.krml obj/FStar_Seq_Base.krml obj/FStar_Seq_Properties.krml obj/FStar_Seq.krml obj/FStar_Math_Lib.krml obj/FStar_Math_Lemmas.krml obj/FStar_BitVector.krml obj/FStar_UInt.krml obj/FStar_UInt32.krml obj/FStar_UInt64.krml obj/FStar_UInt8.krml obj/FStar_Exn.krml obj/FStar_Set.krml obj/FStar_Monotonic_Witnessed.krml obj/FStar_Ghost.krml obj/FStar_ErasedLogic.krml obj/FStar_PropositionalExtensionality.krml obj/FStar_PredicateExtensionality.krml obj/FStar_TSet.krml obj/FStar_Monotonic_Heap.krml obj/FStar_Heap.krml obj/FStar_ST.krml obj/FStar_All.krml obj/FStar_IO.krml obj/FStar_Map.krml obj/Vale_Lib_Seqs_s.krml obj/Vale_Def_Words_s.krml obj/Vale_Def_Words_Four_s.krml obj/Vale_Def_Words_Two_s.krml obj/Vale_Def_Words_Seq_s.krml obj/Vale_Def_Opaque_s.krml obj/Vale_Def_Types_s.krml obj/Vale_X64_Machine_s.krml obj/Vale_Lib_Map16.krml obj/Vale_Def_Prop_s.krml obj/Vale_X64_Xmms.krml obj/Vale_X64_Regs.krml obj/Vale_X64_Instruction_s.krml obj/Vale_Lib_Meta.krml obj/Vale_Def_Words_Two.krml obj/Vale_Lib_Seqs.krml obj/Vale_Def_TypesNative_s.krml obj/Vale_Arch_TypesNative.krml obj/Vale_Def_Words_Seq.krml obj/Vale_Arch_Types.krml obj/FStar_Option.krml obj/Vale_Lib_Set.krml obj/Vale_X64_Bytes_Code_s.krml obj/Vale_AES_AES_s.krml obj/Vale_Math_Poly2_Defs_s.krml obj/Vale_Math_Poly2_s.krml obj/Vale_Math_Poly2_Bits_s.krml obj/FStar_Monotonic_HyperHeap.krml obj/FStar_Monotonic_HyperStack.krml obj/FStar_HyperStack.krml obj/FStar_HyperStack_ST.krml obj/FStar_HyperStack_All.krml obj/FStar_Kremlin_Endianness.krml obj/FStar_Int.krml obj/FStar_Int64.krml obj/FStar_Int63.krml obj/FStar_Int32.krml obj/FStar_Int16.krml obj/FStar_Int8.krml obj/FStar_UInt63.krml obj/FStar_UInt16.krml obj/FStar_Int_Cast.krml obj/FStar_UInt128.krml obj/Spec_Hash_Definitions.krml obj/Spec_Hash_Lemmas0.krml obj/Spec_Hash_PadFinish.krml obj/Spec_Loops.krml obj/FStar_List.krml obj/Spec_SHA2_Constants.krml obj/Spec_SHA2.krml obj/Vale_X64_CryptoInstructions_s.krml obj/Vale_X64_CPU_Features_s.krml obj/Vale_X64_Instructions_s.krml obj/Vale_X64_Machine_Semantics_s.krml obj/Vale_X64_Bytes_Semantics.krml obj/FStar_Universe.krml obj/FStar_GSet.krml obj/FStar_ModifiesGen.krml obj/FStar_Range.krml obj/FStar_Reflection_Types.krml obj/FStar_Tactics_Types.krml obj/FStar_Tactics_Result.krml obj/FStar_Tactics_Effect.krml obj/FStar_Tactics_Util.krml obj/FStar_Reflection_Data.krml obj/FStar_Reflection_Const.krml obj/FStar_Char.krml obj/FStar_String.krml obj/FStar_Order.krml obj/FStar_Reflection_Basic.krml obj/FStar_Reflection_Derived.krml obj/FStar_Tactics_Builtins.krml obj/FStar_Reflection_Formula.krml obj/FStar_Reflection_Derived_Lemmas.krml obj/FStar_Reflection.krml obj/FStar_Tactics_Derived.krml obj/FStar_Tactics_Logic.krml obj/FStar_Tactics.krml obj/FStar_BigOps.krml obj/LowStar_Monotonic_Buffer.krml obj/LowStar_BufferView_Down.krml obj/LowStar_BufferView_Up.krml obj/Vale_Interop_Views.krml obj/LowStar_Buffer.krml obj/LowStar_Modifies.krml obj/LowStar_ModifiesPat.krml obj/LowStar_BufferView.krml obj/Vale_Lib_BufferViewHelpers.krml obj/LowStar_ImmutableBuffer.krml obj/Vale_Interop_Types.krml obj/Vale_Interop_Base.krml obj/Vale_Interop.krml obj/Vale_X64_Memory.krml obj/Vale_X64_Stack_i.krml obj/Vale_X64_Stack_Sems.krml obj/Vale_X64_BufferViewStore.krml obj/Vale_X64_Memory_Sems.krml obj/Vale_X64_State.krml obj/Vale_X64_StateLemmas.krml obj/Vale_X64_Lemmas.krml obj/Vale_X64_Print_s.krml obj/Vale_X64_Decls.krml obj/Vale_X64_Taint_Semantics.krml obj/Vale_X64_InsLemmas.krml obj/Vale_X64_QuickCode.krml obj/Vale_X64_InsAes.krml obj/Vale_Curve25519_Fast_lemmas_internal.krml obj/Vale_Curve25519_Fast_defs.krml obj/FStar_Tactics_CanonCommSwaps.krml obj/FStar_Algebra_CommMonoid.krml obj/FStar_Tactics_CanonCommMonoid.krml obj/FStar_Tactics_CanonCommSemiring.krml obj/Vale_Curve25519_FastUtil_helpers.krml obj/Vale_Curve25519_FastHybrid_helpers.krml obj/Vale_Curve25519_Fast_lemmas_external.krml obj/Vale_X64_QuickCodes.krml obj/Vale_X64_InsBasic.krml obj/Vale_X64_InsMem.krml obj/Vale_X64_InsVector.krml obj/Vale_X64_InsStack.krml obj/Vale_Curve25519_X64_FastHybrid.krml obj/Vale_Curve25519_FastSqr_helpers.krml obj/Vale_Curve25519_X64_FastSqr.krml obj/Vale_Curve25519_FastMul_helpers.krml obj/Vale_Curve25519_X64_FastMul.krml obj/Vale_Curve25519_X64_FastWide.krml obj/Vale_Curve25519_X64_FastUtil.krml obj/Vale_X64_MemoryAdapters.krml obj/Vale_Interop_Assumptions.krml obj/Vale_Interop_X64.krml obj/Vale_AsLowStar_ValeSig.krml obj/Vale_AsLowStar_LowStarSig.krml obj/Vale_AsLowStar_MemoryHelpers.krml obj/Vale_AsLowStar_Wrapper.krml obj/Vale_Stdcalls_X64_Fadd.krml obj/Vale_Wrapper_X64_Fadd.krml obj/Vale_Math_Poly2_Defs.krml obj/Vale_Math_Poly2.krml obj/Vale_Math_Poly2_Lemmas.krml obj/Vale_Math_Poly2_Bits.krml obj/Vale_Math_Poly2_Words.krml obj/Vale_AES_GF128_s.krml obj/Vale_AES_GF128.krml obj/Vale_AES_OptPublic.krml obj/Vale_AES_X64_GF128_Mul.krml obj/Vale_AES_X64_PolyOps.krml obj/Vale_X64_Stack.krml obj/Vale_AES_GCTR_s.krml obj/Vale_Lib_Workarounds.krml obj/Vale_AES_GCM_helpers.krml obj/Vale_AES_GCTR.krml obj/Vale_AES_AES256_helpers.krml obj/Vale_AES_X64_AES256.krml obj/Vale_AES_AES_helpers.krml obj/Vale_AES_X64_AES128.krml obj/Vale_AES_X64_AES.krml obj/FStar_BV.krml obj/Vale_Lib_Bv_s.krml obj/Vale_Math_Bits.krml obj/Vale_AES_GHash_s.krml obj/Vale_AES_GHash.krml obj/Vale_AES_X64_GF128_Init.krml obj/Vale_Lib_Operator.krml obj/Lib_LoopCombinators.krml obj/FStar_Int_Cast_Full.krml obj/Lib_IntTypes.krml obj/Lib_Sequence.krml obj/Spec_SHA3_Constants.krml obj/Spec_SHA1.krml obj/Spec_MD5.krml obj/Spec_Hash.krml obj/Spec_Hash_Incremental.krml obj/Spec_Hash_Lemmas.krml obj/LowStar_BufferOps.krml obj/C_Loops.krml obj/C_Endianness.krml obj/Hacl_Hash_Lemmas.krml obj/Hacl_Hash_Definitions.krml obj/Hacl_Hash_PadFinish.krml obj/Hacl_Hash_MD.krml obj/Spec_SHA2_Lemmas.krml obj/Vale_SHA_SHA_helpers.krml obj/Vale_X64_InsSha.krml obj/Vale_SHA_X64.krml obj/Vale_Stdcalls_X64_Sha.krml obj/Vale_SHA_Simplify_Sha.krml obj/Vale_Wrapper_X64_Sha.krml obj/Hacl_Hash_Core_SHA2_Constants.krml obj/Hacl_Hash_Core_SHA2.krml obj/Lib_RawIntTypes.krml obj/Lib_Loops.krml obj/Lib_ByteSequence.krml obj/Lib_Buffer.krml obj/FStar_Reflection_Arith.krml obj/FStar_Tactics_BV.krml obj/Vale_Lib_Tactics.krml obj/Vale_Poly1305_Bitvectors.krml obj/Vale_Math_Lemmas_Int.krml obj/FStar_Tactics_Canon.krml obj/Vale_Poly1305_Spec_s.krml obj/Vale_Poly1305_Math.krml obj/Vale_Poly1305_Util.krml obj/Vale_Poly1305_X64.krml obj/Vale_Stdcalls_X64_Poly.krml obj/Vale_Wrapper_X64_Poly.krml obj/FStar_Endianness.krml obj/Vale_Arch_BufferFriend.krml obj/Hacl_Hash_SHA2.krml obj/Hacl_Hash_Core_SHA1.krml obj/Hacl_Hash_SHA1.krml obj/Hacl_Hash_Core_MD5.krml obj/Hacl_Hash_MD5.krml obj/C.krml obj/C_String.krml obj/C_Failure.krml obj/FStar_Int128.krml obj/FStar_Int31.krml obj/FStar_UInt31.krml obj/FStar_Integers.krml obj/EverCrypt_StaticConfig.krml obj/Vale_Lib_X64_Cpuid.krml obj/Vale_Lib_X64_Cpuidstdcall.krml obj/Vale_Stdcalls_X64_Cpuid.krml obj/Vale_Wrapper_X64_Cpuid.krml obj/EverCrypt_TargetConfig.krml obj/EverCrypt_AutoConfig2.krml obj/EverCrypt_Helpers.krml obj/EverCrypt_Hash.krml obj/Hacl_Impl_Curve25519_Lemmas.krml obj/Spec_Curve25519_Lemmas.krml obj/Spec_Curve25519.krml obj/Hacl_Spec_Curve25519_Field51_Definition.krml obj/Hacl_Spec_Curve25519_Field51_Lemmas.krml obj/Hacl_Spec_Curve25519_Field51.krml obj/Hacl_Impl_Curve25519_Field51.krml obj/Hacl_Spec_Curve25519_Finv.krml obj/Hacl_Spec_Curve25519_Field64_Definition.krml obj/Hacl_Spec_Curve25519_Field64_Lemmas.krml obj/Hacl_Spec_Curve25519_Field64_Core.krml obj/Hacl_Spec_Curve25519_Field64.krml obj/Vale_Stdcalls_X64_Fswap.krml obj/Vale_Wrapper_X64_Fswap.krml obj/Vale_X64_Print_Inline_s.krml obj/Vale_Inline_X64_Fswap_inline.krml obj/Vale_Stdcalls_X64_Fsqr.krml obj/Vale_Wrapper_X64_Fsqr.krml obj/Vale_Inline_X64_Fsqr_inline.krml obj/Vale_Stdcalls_X64_Fmul.krml obj/Vale_Wrapper_X64_Fmul.krml obj/Vale_Inline_X64_Fmul_inline.krml obj/Vale_Stdcalls_X64_Fsub.krml obj/Vale_Wrapper_X64_Fsub.krml obj/Vale_Inline_X64_Fadd_inline.krml obj/Hacl_Impl_Curve25519_Field64_Core.krml obj/Hacl_Impl_Curve25519_Field64.krml obj/Hacl_Impl_Curve25519_Fields.krml obj/Lib_ByteBuffer.krml obj/Hacl_Impl_Curve25519_Finv.krml obj/Hacl_Curve25519_Finv_Field51.krml obj/Hacl_Bignum25519.krml obj/Hacl_Impl_Ed25519_PointAdd.krml obj/Hacl_Impl_Ed25519_PointDouble.krml obj/Hacl_Impl_Ed25519_SwapConditional.krml obj/Hacl_Impl_Ed25519_Ladder.krml obj/Hacl_Impl_Ed25519_PointCompress.krml obj/Hacl_Impl_Ed25519_SecretExpand.krml obj/Hacl_Impl_Ed25519_SecretToPublic.krml obj/Hacl_Impl_BignumQ_Mul.krml obj/Hacl_Impl_Load56.krml obj/Hacl_Impl_Store56.krml obj/Hacl_Impl_SHA512_ModQ.krml obj/Hacl_Impl_Ed25519_Sign_Steps.krml obj/Hacl_Impl_Ed25519_Sign.krml obj/Hacl_Impl_Ed25519_Sign_Expanded.krml obj/LowStar_Vector.krml obj/LowStar_Regional.krml obj/LowStar_RVector.krml obj/Vale_AES_GCM_s.krml obj/Vale_AES_GCM.krml obj/Hacl_Spec_Curve25519_AddAndDouble.krml obj/Hacl_Impl_Curve25519_AddAndDouble.krml obj/Lib_IntVector_Intrinsics.krml obj/Lib_IntVector.krml obj/Vale_AES_Gcm_simplify.krml obj/Vale_AES_X64_GHash.krml obj/Vale_AES_X64_AESCTR.krml obj/Vale_AES_X64_AESCTRplain.krml obj/Vale_AES_X64_GCTR.krml obj/Vale_AES_X64_GCMencrypt.krml obj/Vale_AES_X64_GCMdecrypt.krml obj/Vale_Stdcalls_X64_GCMdecrypt.krml obj/Vale_Stdcalls_X64_Aes.krml obj/Vale_Wrapper_X64_AES.krml obj/Vale_Wrapper_X64_GCMdecrypt.krml obj/Spec_Chacha20.krml obj/Hacl_Impl_Chacha20_Core32.krml obj/Hacl_Impl_Chacha20.krml obj/Hacl_Spec_Poly1305_Vec.krml obj/Hacl_Impl_Poly1305_Lemmas.krml obj/Hacl_Spec_Poly1305_Field32xN.krml obj/Hacl_Poly1305_Field32xN_Lemmas.krml obj/Hacl_Spec_Poly1305_Field32xN_Lemmas.krml obj/Hacl_Impl_Poly1305_Field32xN.krml obj/Hacl_Impl_Poly1305_Fields.krml obj/Spec_Poly1305.krml obj/Hacl_Spec_Poly1305_Equiv_Lemmas.krml obj/Hacl_Spec_Poly1305_Equiv.krml obj/Hacl_Impl_Poly1305.krml obj/Hacl_Poly1305_32.krml obj/Spec_Chacha20Poly1305.krml obj/Hacl_Impl_Chacha20Poly1305_PolyCore.krml obj/Hacl_Impl_Chacha20Poly1305_Poly.krml obj/Hacl_Impl_Chacha20Poly1305.krml obj/FStar_Dyn.krml obj/EverCrypt_Vale.krml obj/EverCrypt_Specs.krml obj/EverCrypt_OpenSSL.krml obj/EverCrypt_Hacl.krml obj/EverCrypt_BCrypt.krml obj/EverCrypt_Cipher.krml obj/Hacl_Impl_Curve25519_Generic.krml obj/Hacl_Curve25519_51.krml obj/Hacl_Curve25519_64.krml obj/EverCrypt_Curve25519.krml obj/Hacl_Poly1305_128.krml obj/Hacl_Poly1305_256.krml obj/Vale_Poly1305_Equiv.krml obj/Vale_Poly1305_CallingFromLowStar.krml obj/EverCrypt_Poly1305.krml obj/Spec_HMAC.krml obj/Hacl_HMAC.krml obj/EverCrypt_HMAC.krml obj/Spec_HKDF.krml obj/EverCrypt_HKDF.krml obj/EverCrypt.krml obj/MerkleTree_Spec.krml obj/MerkleTree_New_High.krml obj/LowStar_Regional_Instances.krml obj/MerkleTree_New_Low.krml obj/MerkleTree_New_Low_Serialization.krml obj/Vale_AES_X64_AESopt2.krml obj/Vale_AES_X64_AESGCM.krml obj/Vale_AES_X64_GCMencryptOpt.krml obj/Vale_AES_X64_GCMdecryptOpt.krml obj/Vale_Stdcalls_X64_GCMdecryptOpt.krml obj/Vale_Wrapper_X64_GCMdecryptOpt.krml obj/Vale_Wrapper_X64_GCMdecryptOpt256.krml obj/Vale_Stdcalls_X64_GCMencryptOpt.krml obj/Vale_Wrapper_X64_GCMencryptOpt.krml obj/Vale_Wrapper_X64_GCMencryptOpt256.krml obj/Vale_Stdcalls_X64_AesHash.krml obj/Vale_Wrapper_X64_AEShash.krml obj/Spec_AEAD.krml obj/EverCrypt_Error.krml obj/EverCrypt_AEAD.krml obj/WasmSupport.krml obj/MerkleTree_New_High_Correct_Base.krml obj/MerkleTree_New_High_Correct_Rhs.krml obj/MerkleTree_New_High_Correct_Path.krml obj/MerkleTree_New_High_Correct_Flushing.krml obj/MerkleTree_New_High_Correct_Insertion.krml obj/MerkleTree_New_High_Correct.krml obj/EverCrypt_Hash_Incremental.krml obj/Test_Hash.krml obj/Spec_SHA3.krml obj/Hacl_Impl_SHA3.krml obj/TestLib.krml obj/EverCrypt_Chacha20Poly1305.krml obj/Hacl_SHA3.krml obj/Lib_PrintBuffer.krml obj/Hacl_Test_CSHAKE.krml obj/Spec_Hash_Test.krml obj/Hacl_Impl_Ed25519_Pow2_252m2.krml obj/Hacl_Impl_Ed25519_RecoverX.krml obj/Hacl_Impl_Ed25519_PointDecompress.krml obj/Hacl_Impl_Ed25519_PointEqual.krml obj/Hacl_Impl_Ed25519_Verify.krml obj/Hacl_Ed25519.krml obj/Hacl_Test_Ed25519.krml obj/Vale_Stdcalls_X64_GCTR.krml obj/Vale_X64_Leakage_s.krml obj/Vale_Wrapper_X64_GCTR.krml obj/Test_Vectors_Chacha20Poly1305.krml obj/Lib_RandomBuffer.krml obj/Test_Vectors_Aes128.krml obj/Vale_Test_X64_Args.krml obj/Vale_Stdcalls_X64_GCMencrypt.krml obj/LowStar_Endianness.krml obj/Hacl_Hash_Agile.krml obj/Vale_Test_X64_Vale_memcpy.krml obj/Hacl_Test_SHA3.krml obj/Test_Vectors_Curve25519.krml obj/Vale_Test_X64_Memcpy.krml obj/Spec_Chacha20Poly1305_Test.krml obj/Vale_Lib_Lists.krml obj/Vale_Wrapper_X64_GCMencrypt.krml obj/Vale_AES_X64_AESopt.krml obj/Test_Vectors_Poly1305.krml obj/Vale_AsLowStar_Test.krml obj/Vale_X64_Leakage_Helpers.krml obj/Vale_X64_Leakage_Ins.krml obj/Vale_X64_Leakage.krml obj/Spec_SHA3_Test.krml obj/Test_Lowstarize.krml obj/Test_Vectors.krml obj/Test_NoHeap.krml obj/Test_Vectors_Aes128Gcm.krml obj/Spec_Curve25519_Test.krml obj/Test.krml obj/Spec_Chacha20_Test.krml -silent -ccopt -Wno-unused -warn-error @4-6 -fparentheses Hacl_AES.c Lib_RandomBuffer.c Lib_PrintBuffer.c evercrypt_vale_stubs.c -o libevercrypt.a
  F* version: 7c70b890
  KreMLin version: b511d90c
 */

#include "Hacl_Chacha20.h"

inline static void Hacl_Impl_Chacha20_Core32_double_round(uint32_t *st)
{
  uint32_t sta = st[0U];
  uint32_t stb0 = st[4U];
  uint32_t std0 = st[12U];
  uint32_t sta10 = sta + stb0;
  uint32_t std10 = std0 ^ sta10;
  uint32_t std2 = std10 << (uint32_t)16U | std10 >> (uint32_t)16U;
  st[0U] = sta10;
  st[12U] = std2;
  uint32_t sta0 = st[8U];
  uint32_t stb1 = st[12U];
  uint32_t std3 = st[4U];
  uint32_t sta11 = sta0 + stb1;
  uint32_t std11 = std3 ^ sta11;
  uint32_t std20 = std11 << (uint32_t)12U | std11 >> (uint32_t)20U;
  st[8U] = sta11;
  st[4U] = std20;
  uint32_t sta2 = st[0U];
  uint32_t stb2 = st[4U];
  uint32_t std4 = st[12U];
  uint32_t sta12 = sta2 + stb2;
  uint32_t std12 = std4 ^ sta12;
  uint32_t std21 = std12 << (uint32_t)8U | std12 >> (uint32_t)24U;
  st[0U] = sta12;
  st[12U] = std21;
  uint32_t sta3 = st[8U];
  uint32_t stb3 = st[12U];
  uint32_t std5 = st[4U];
  uint32_t sta13 = sta3 + stb3;
  uint32_t std13 = std5 ^ sta13;
  uint32_t std22 = std13 << (uint32_t)7U | std13 >> (uint32_t)25U;
  st[8U] = sta13;
  st[4U] = std22;
  uint32_t sta4 = st[1U];
  uint32_t stb4 = st[5U];
  uint32_t std6 = st[13U];
  uint32_t sta14 = sta4 + stb4;
  uint32_t std14 = std6 ^ sta14;
  uint32_t std23 = std14 << (uint32_t)16U | std14 >> (uint32_t)16U;
  st[1U] = sta14;
  st[13U] = std23;
  uint32_t sta5 = st[9U];
  uint32_t stb5 = st[13U];
  uint32_t std7 = st[5U];
  uint32_t sta15 = sta5 + stb5;
  uint32_t std15 = std7 ^ sta15;
  uint32_t std24 = std15 << (uint32_t)12U | std15 >> (uint32_t)20U;
  st[9U] = sta15;
  st[5U] = std24;
  uint32_t sta6 = st[1U];
  uint32_t stb6 = st[5U];
  uint32_t std8 = st[13U];
  uint32_t sta16 = sta6 + stb6;
  uint32_t std16 = std8 ^ sta16;
  uint32_t std25 = std16 << (uint32_t)8U | std16 >> (uint32_t)24U;
  st[1U] = sta16;
  st[13U] = std25;
  uint32_t sta7 = st[9U];
  uint32_t stb7 = st[13U];
  uint32_t std9 = st[5U];
  uint32_t sta17 = sta7 + stb7;
  uint32_t std17 = std9 ^ sta17;
  uint32_t std26 = std17 << (uint32_t)7U | std17 >> (uint32_t)25U;
  st[9U] = sta17;
  st[5U] = std26;
  uint32_t sta8 = st[2U];
  uint32_t stb8 = st[6U];
  uint32_t std18 = st[14U];
  uint32_t sta18 = sta8 + stb8;
  uint32_t std19 = std18 ^ sta18;
  uint32_t std27 = std19 << (uint32_t)16U | std19 >> (uint32_t)16U;
  st[2U] = sta18;
  st[14U] = std27;
  uint32_t sta9 = st[10U];
  uint32_t stb9 = st[14U];
  uint32_t std28 = st[6U];
  uint32_t sta19 = sta9 + stb9;
  uint32_t std110 = std28 ^ sta19;
  uint32_t std29 = std110 << (uint32_t)12U | std110 >> (uint32_t)20U;
  st[10U] = sta19;
  st[6U] = std29;
  uint32_t sta20 = st[2U];
  uint32_t stb10 = st[6U];
  uint32_t std30 = st[14U];
  uint32_t sta110 = sta20 + stb10;
  uint32_t std111 = std30 ^ sta110;
  uint32_t std210 = std111 << (uint32_t)8U | std111 >> (uint32_t)24U;
  st[2U] = sta110;
  st[14U] = std210;
  uint32_t sta21 = st[10U];
  uint32_t stb11 = st[14U];
  uint32_t std31 = st[6U];
  uint32_t sta111 = sta21 + stb11;
  uint32_t std112 = std31 ^ sta111;
  uint32_t std211 = std112 << (uint32_t)7U | std112 >> (uint32_t)25U;
  st[10U] = sta111;
  st[6U] = std211;
  uint32_t sta22 = st[3U];
  uint32_t stb12 = st[7U];
  uint32_t std32 = st[15U];
  uint32_t sta112 = sta22 + stb12;
  uint32_t std113 = std32 ^ sta112;
  uint32_t std212 = std113 << (uint32_t)16U | std113 >> (uint32_t)16U;
  st[3U] = sta112;
  st[15U] = std212;
  uint32_t sta23 = st[11U];
  uint32_t stb13 = st[15U];
  uint32_t std33 = st[7U];
  uint32_t sta113 = sta23 + stb13;
  uint32_t std114 = std33 ^ sta113;
  uint32_t std213 = std114 << (uint32_t)12U | std114 >> (uint32_t)20U;
  st[11U] = sta113;
  st[7U] = std213;
  uint32_t sta24 = st[3U];
  uint32_t stb14 = st[7U];
  uint32_t std34 = st[15U];
  uint32_t sta114 = sta24 + stb14;
  uint32_t std115 = std34 ^ sta114;
  uint32_t std214 = std115 << (uint32_t)8U | std115 >> (uint32_t)24U;
  st[3U] = sta114;
  st[15U] = std214;
  uint32_t sta25 = st[11U];
  uint32_t stb15 = st[15U];
  uint32_t std35 = st[7U];
  uint32_t sta115 = sta25 + stb15;
  uint32_t std116 = std35 ^ sta115;
  uint32_t std215 = std116 << (uint32_t)7U | std116 >> (uint32_t)25U;
  st[11U] = sta115;
  st[7U] = std215;
  uint32_t sta26 = st[0U];
  uint32_t stb16 = st[5U];
  uint32_t std36 = st[15U];
  uint32_t sta116 = sta26 + stb16;
  uint32_t std117 = std36 ^ sta116;
  uint32_t std216 = std117 << (uint32_t)16U | std117 >> (uint32_t)16U;
  st[0U] = sta116;
  st[15U] = std216;
  uint32_t sta27 = st[10U];
  uint32_t stb17 = st[15U];
  uint32_t std37 = st[5U];
  uint32_t sta117 = sta27 + stb17;
  uint32_t std118 = std37 ^ sta117;
  uint32_t std217 = std118 << (uint32_t)12U | std118 >> (uint32_t)20U;
  st[10U] = sta117;
  st[5U] = std217;
  uint32_t sta28 = st[0U];
  uint32_t stb18 = st[5U];
  uint32_t std38 = st[15U];
  uint32_t sta118 = sta28 + stb18;
  uint32_t std119 = std38 ^ sta118;
  uint32_t std218 = std119 << (uint32_t)8U | std119 >> (uint32_t)24U;
  st[0U] = sta118;
  st[15U] = std218;
  uint32_t sta29 = st[10U];
  uint32_t stb19 = st[15U];
  uint32_t std39 = st[5U];
  uint32_t sta119 = sta29 + stb19;
  uint32_t std120 = std39 ^ sta119;
  uint32_t std219 = std120 << (uint32_t)7U | std120 >> (uint32_t)25U;
  st[10U] = sta119;
  st[5U] = std219;
  uint32_t sta30 = st[1U];
  uint32_t stb20 = st[6U];
  uint32_t std40 = st[12U];
  uint32_t sta120 = sta30 + stb20;
  uint32_t std121 = std40 ^ sta120;
  uint32_t std220 = std121 << (uint32_t)16U | std121 >> (uint32_t)16U;
  st[1U] = sta120;
  st[12U] = std220;
  uint32_t sta31 = st[11U];
  uint32_t stb21 = st[12U];
  uint32_t std41 = st[6U];
  uint32_t sta121 = sta31 + stb21;
  uint32_t std122 = std41 ^ sta121;
  uint32_t std221 = std122 << (uint32_t)12U | std122 >> (uint32_t)20U;
  st[11U] = sta121;
  st[6U] = std221;
  uint32_t sta32 = st[1U];
  uint32_t stb22 = st[6U];
  uint32_t std42 = st[12U];
  uint32_t sta122 = sta32 + stb22;
  uint32_t std123 = std42 ^ sta122;
  uint32_t std222 = std123 << (uint32_t)8U | std123 >> (uint32_t)24U;
  st[1U] = sta122;
  st[12U] = std222;
  uint32_t sta33 = st[11U];
  uint32_t stb23 = st[12U];
  uint32_t std43 = st[6U];
  uint32_t sta123 = sta33 + stb23;
  uint32_t std124 = std43 ^ sta123;
  uint32_t std223 = std124 << (uint32_t)7U | std124 >> (uint32_t)25U;
  st[11U] = sta123;
  st[6U] = std223;
  uint32_t sta34 = st[2U];
  uint32_t stb24 = st[7U];
  uint32_t std44 = st[13U];
  uint32_t sta124 = sta34 + stb24;
  uint32_t std125 = std44 ^ sta124;
  uint32_t std224 = std125 << (uint32_t)16U | std125 >> (uint32_t)16U;
  st[2U] = sta124;
  st[13U] = std224;
  uint32_t sta35 = st[8U];
  uint32_t stb25 = st[13U];
  uint32_t std45 = st[7U];
  uint32_t sta125 = sta35 + stb25;
  uint32_t std126 = std45 ^ sta125;
  uint32_t std225 = std126 << (uint32_t)12U | std126 >> (uint32_t)20U;
  st[8U] = sta125;
  st[7U] = std225;
  uint32_t sta36 = st[2U];
  uint32_t stb26 = st[7U];
  uint32_t std46 = st[13U];
  uint32_t sta126 = sta36 + stb26;
  uint32_t std127 = std46 ^ sta126;
  uint32_t std226 = std127 << (uint32_t)8U | std127 >> (uint32_t)24U;
  st[2U] = sta126;
  st[13U] = std226;
  uint32_t sta37 = st[8U];
  uint32_t stb27 = st[13U];
  uint32_t std47 = st[7U];
  uint32_t sta127 = sta37 + stb27;
  uint32_t std128 = std47 ^ sta127;
  uint32_t std227 = std128 << (uint32_t)7U | std128 >> (uint32_t)25U;
  st[8U] = sta127;
  st[7U] = std227;
  uint32_t sta38 = st[3U];
  uint32_t stb28 = st[4U];
  uint32_t std48 = st[14U];
  uint32_t sta128 = sta38 + stb28;
  uint32_t std129 = std48 ^ sta128;
  uint32_t std228 = std129 << (uint32_t)16U | std129 >> (uint32_t)16U;
  st[3U] = sta128;
  st[14U] = std228;
  uint32_t sta39 = st[9U];
  uint32_t stb29 = st[14U];
  uint32_t std49 = st[4U];
  uint32_t sta129 = sta39 + stb29;
  uint32_t std130 = std49 ^ sta129;
  uint32_t std229 = std130 << (uint32_t)12U | std130 >> (uint32_t)20U;
  st[9U] = sta129;
  st[4U] = std229;
  uint32_t sta40 = st[3U];
  uint32_t stb30 = st[4U];
  uint32_t std50 = st[14U];
  uint32_t sta130 = sta40 + stb30;
  uint32_t std131 = std50 ^ sta130;
  uint32_t std230 = std131 << (uint32_t)8U | std131 >> (uint32_t)24U;
  st[3U] = sta130;
  st[14U] = std230;
  uint32_t sta41 = st[9U];
  uint32_t stb = st[14U];
  uint32_t std = st[4U];
  uint32_t sta1 = sta41 + stb;
  uint32_t std1 = std ^ sta1;
  uint32_t std231 = std1 << (uint32_t)7U | std1 >> (uint32_t)25U;
  st[9U] = sta1;
  st[4U] = std231;
}

inline void Hacl_Impl_Chacha20_rounds(uint32_t *st)
{
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
  Hacl_Impl_Chacha20_Core32_double_round(st);
}

inline void Hacl_Impl_Chacha20_chacha20_core(uint32_t *k, uint32_t *ctx, uint32_t ctr)
{
  memcpy(k, ctx, (uint32_t)16U * sizeof ctx[0U]);
  uint32_t ctr_u32 = ctr;
  k[12U] = k[12U] + ctr_u32;
  Hacl_Impl_Chacha20_rounds(k);
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint32_t *os = k;
    uint32_t x = k[i] + ctx[i];
    os[i] = x;
  }
  k[12U] = k[12U] + ctr_u32;
}

uint32_t
Hacl_Impl_Chacha20_chacha20_constants[4U] =
  { (uint32_t)0x61707865U, (uint32_t)0x3320646eU, (uint32_t)0x79622d32U, (uint32_t)0x6b206574U };

inline void
Hacl_Impl_Chacha20_chacha20_init(uint32_t *ctx, uint8_t *k, uint8_t *n1, uint32_t ctr)
{
  uint32_t *uu____0 = ctx;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)4U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____0;
    uint32_t x = Hacl_Impl_Chacha20_chacha20_constants[i];
    os[i] = x;
  }
  uint32_t *uu____1 = ctx + (uint32_t)4U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)8U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____1;
    uint8_t *bj = k + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  ctx[12U] = ctr;
  uint32_t *uu____2 = ctx + (uint32_t)13U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)3U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____2;
    uint8_t *bj = n1 + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
}

inline void
Hacl_Impl_Chacha20_chacha20_encrypt_block(
  uint32_t *ctx,
  uint8_t *out,
  uint32_t incr1,
  uint8_t *text
)
{
  uint32_t k[16U] = { 0U };
  Hacl_Impl_Chacha20_chacha20_core(k, ctx, incr1);
  uint32_t bl[16U] = { 0U };
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint32_t *os = bl;
    uint8_t *bj = text + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint32_t *os = bl;
    uint32_t x = bl[i] ^ k[i];
    os[i] = x;
  }
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    store32_le(out + i * (uint32_t)4U, bl[i]);
  }
}

inline void
Hacl_Impl_Chacha20_chacha20_encrypt_last(
  uint32_t *ctx,
  uint32_t len1,
  uint8_t *out,
  uint32_t incr1,
  uint8_t *text
)
{
  uint8_t plain[64U] = { 0U };
  memcpy(plain, text, len1 * sizeof text[0U]);
  uint32_t k[16U] = { 0U };
  Hacl_Impl_Chacha20_chacha20_core(k, ctx, incr1);
  uint32_t bl[16U] = { 0U };
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint32_t *os = bl;
    uint8_t *bj = plain + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint32_t *os = bl;
    uint32_t x = bl[i] ^ k[i];
    os[i] = x;
  }
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    store32_le(plain + i * (uint32_t)4U, bl[i]);
  }
  memcpy(out, plain, len1 * sizeof plain[0U]);
}

inline void
Hacl_Impl_Chacha20_chacha20_update(uint32_t *ctx, uint32_t len1, uint8_t *out, uint8_t *text)
{
  uint32_t k[16U] = { 0U };
  uint32_t rem1 = len1 % (uint32_t)64U;
  uint32_t nb = len1 / (uint32_t)64U;
  uint32_t rem2 = len1 % (uint32_t)64U;
  for (uint32_t i0 = (uint32_t)0U; i0 < nb; i0 = i0 + (uint32_t)1U)
  {
    uint8_t *uu____0 = out + i0 * (uint32_t)64U;
    uint8_t *uu____1 = text + i0 * (uint32_t)64U;
    uint32_t k1[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k1, ctx, i0);
    uint32_t bl[16U] = { 0U };
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint8_t *bj = uu____1 + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint32_t x = bl[i] ^ k1[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(uu____0 + i * (uint32_t)4U, bl[i]);
    }
  }
  if (rem2 > (uint32_t)0U)
  {
    uint8_t *uu____2 = out + nb * (uint32_t)64U;
    uint8_t *uu____3 = text + nb * (uint32_t)64U;
    uint8_t plain[64U] = { 0U };
    memcpy(plain, uu____3, rem1 * sizeof uu____3[0U]);
    uint32_t k1[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k1, ctx, nb);
    uint32_t bl[16U] = { 0U };
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint8_t *bj = plain + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint32_t x = bl[i] ^ k1[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(plain + i * (uint32_t)4U, bl[i]);
    }
    memcpy(uu____2, plain, rem1 * sizeof plain[0U]);
  }
}

void
Hacl_Impl_Chacha20_chacha20_encrypt(
  uint32_t len1,
  uint8_t *out,
  uint8_t *text,
  uint8_t *key,
  uint8_t *n1,
  uint32_t ctr
)
{
  uint32_t ctx[16U] = { 0U };
  uint32_t *uu____0 = ctx;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)4U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____0;
    uint32_t x = Hacl_Impl_Chacha20_chacha20_constants[i];
    os[i] = x;
  }
  uint32_t *uu____1 = ctx + (uint32_t)4U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)8U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____1;
    uint8_t *bj = key + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  ctx[12U] = ctr;
  uint32_t *uu____2 = ctx + (uint32_t)13U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)3U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____2;
    uint8_t *bj = n1 + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  uint32_t k[16U] = { 0U };
  uint32_t rem1 = len1 % (uint32_t)64U;
  uint32_t nb = len1 / (uint32_t)64U;
  uint32_t rem2 = len1 % (uint32_t)64U;
  for (uint32_t i0 = (uint32_t)0U; i0 < nb; i0 = i0 + (uint32_t)1U)
  {
    uint8_t *uu____3 = out + i0 * (uint32_t)64U;
    uint8_t *uu____4 = text + i0 * (uint32_t)64U;
    uint32_t k1[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k1, ctx, i0);
    uint32_t bl[16U] = { 0U };
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint8_t *bj = uu____4 + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint32_t x = bl[i] ^ k1[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(uu____3 + i * (uint32_t)4U, bl[i]);
    }
  }
  if (rem2 > (uint32_t)0U)
  {
    uint8_t *uu____5 = out + nb * (uint32_t)64U;
    uint8_t *uu____6 = text + nb * (uint32_t)64U;
    uint8_t plain[64U] = { 0U };
    memcpy(plain, uu____6, rem1 * sizeof uu____6[0U]);
    uint32_t k1[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k1, ctx, nb);
    uint32_t bl[16U] = { 0U };
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint8_t *bj = plain + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint32_t x = bl[i] ^ k1[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(plain + i * (uint32_t)4U, bl[i]);
    }
    memcpy(uu____5, plain, rem1 * sizeof plain[0U]);
  }
}

void
Hacl_Impl_Chacha20_chacha20_decrypt(
  uint32_t len1,
  uint8_t *out,
  uint8_t *cipher,
  uint8_t *key,
  uint8_t *n1,
  uint32_t ctr
)
{
  uint32_t ctx[16U] = { 0U };
  uint32_t *uu____0 = ctx;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)4U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____0;
    uint32_t x = Hacl_Impl_Chacha20_chacha20_constants[i];
    os[i] = x;
  }
  uint32_t *uu____1 = ctx + (uint32_t)4U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)8U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____1;
    uint8_t *bj = key + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  ctx[12U] = ctr;
  uint32_t *uu____2 = ctx + (uint32_t)13U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)3U; i = i + (uint32_t)1U)
  {
    uint32_t *os = uu____2;
    uint8_t *bj = n1 + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  uint32_t k[16U] = { 0U };
  uint32_t rem1 = len1 % (uint32_t)64U;
  uint32_t nb = len1 / (uint32_t)64U;
  uint32_t rem2 = len1 % (uint32_t)64U;
  for (uint32_t i0 = (uint32_t)0U; i0 < nb; i0 = i0 + (uint32_t)1U)
  {
    uint8_t *uu____3 = out + i0 * (uint32_t)64U;
    uint8_t *uu____4 = cipher + i0 * (uint32_t)64U;
    uint32_t k1[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k1, ctx, i0);
    uint32_t bl[16U] = { 0U };
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint8_t *bj = uu____4 + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint32_t x = bl[i] ^ k1[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(uu____3 + i * (uint32_t)4U, bl[i]);
    }
  }
  if (rem2 > (uint32_t)0U)
  {
    uint8_t *uu____5 = out + nb * (uint32_t)64U;
    uint8_t *uu____6 = cipher + nb * (uint32_t)64U;
    uint8_t plain[64U] = { 0U };
    memcpy(plain, uu____6, rem1 * sizeof uu____6[0U]);
    uint32_t k1[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k1, ctx, nb);
    uint32_t bl[16U] = { 0U };
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint8_t *bj = plain + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      uint32_t *os = bl;
      uint32_t x = bl[i] ^ k1[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(plain + i * (uint32_t)4U, bl[i]);
    }
    memcpy(uu____5, plain, rem1 * sizeof plain[0U]);
  }
}


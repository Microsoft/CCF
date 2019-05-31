/* 
  This file was generated by KreMLin <https://github.com/FStarLang/kremlin>
  KreMLin invocation: /data/everest/everest/kremlin/krml -bundle Hacl.Spec.*,Spec.*[rename=Hacl_Spec] -bundle Hacl.Poly1305.Field32xN.Lemmas[rename=Hacl_Lemmas] -bundle Lib.*[rename=Hacl_Lib] -drop Lib.IntVector.Intrinsics -add-include "libintvector.h" -add-include "evercrypt_targetconfig.h" -drop EverCrypt.TargetConfig -bundle EverCrypt.BCrypt -bundle EverCrypt.OpenSSL -bundle MerkleTree.Spec,MerkleTree.Spec.*,MerkleTree.New.High,MerkleTree.New.High.* -bundle Vale.Stdcalls.*,Vale.Interop,Vale.Interop.*,Vale.Wrapper.X64.*[rename=Vale] -bundle Vale.Inline.X64.*[rename=Vale_Inline] -bundle FStar.Tactics.CanonCommMonoid,FStar.Tactics.CanonCommSemiring,FStar.Tactics.CanonCommSwaps[rename=Unused] -bundle Vale.*[rename=Unused2] -library Vale.Stdcalls.* -static-header Vale_Inline -library Vale.Inline.X64.Fadd_inline -library Vale.Inline.X64.Fmul_inline -library Vale.Inline.X64.Fswap_inline -library Vale.Inline.X64.Fsqr_inline -no-prefix Vale.Stdcalls.* -no-prefix Vale.Inline.X64.Fadd_inline -no-prefix Vale.Inline.X64.Fmul_inline -no-prefix Vale.Inline.X64.Fswap_inline -no-prefix Vale.Inline.X64.Fsqr_inline -no-prefix EverCrypt.Vale -add-include "curve25519-inline.h" -no-prefix MerkleTree.New.Low -no-prefix MerkleTree.New.Low.Serialization -fparentheses -fno-shadow -fcurly-braces -bundle WasmSupport -bundle Test,Test.*,Hacl.Test.* -bundle Hacl.Hash.MD5+Hacl.Hash.Core.MD5+Hacl.Hash.SHA1+Hacl.Hash.Core.SHA1+Hacl.Hash.SHA2+Hacl.Hash.Core.SHA2+Hacl.Hash.Core.SHA2.Constants=Hacl.Hash.*[rename=Hacl_Hash] -bundle Hacl.Impl.SHA3+Hacl.SHA3=[rename=Hacl_SHA3] -bundle Hacl.Impl.Poly1305.*[rename=Unused_Poly1305] -bundle Hacl.Impl.Chacha20=Hacl.Impl.Chacha20.*[rename=Hacl_Chacha20] -bundle Hacl.Curve25519_51+Hacl.Curve25519_64=Hacl.Impl.Curve25519.*[rename=Hacl_Curve25519] -bundle Hacl.Impl.Chacha20Poly1305=Hacl.Impl.Chacha20Poly1305.*[rename=Hacl_Chacha20Poly1305] -bundle Hacl.Ed25519=Hacl.Impl.Ed25519.*,Hacl.Impl.BignumQ.Mul,Hacl.Impl.Load56,Hacl.Impl.SHA512.ModQ,Hacl.Impl.Store56,Hacl.Bignum25519 -bundle LowStar.* -bundle Prims,C.Failure,C,C.String,C.Loops,Spec.Loops,C.Endianness,FStar.*[rename=Hacl_Kremlib] -bundle EverCrypt.Spec.* -bundle MerkleTree.New.Low+MerkleTree.New.Low.Serialization=[rename=MerkleTree] -bundle Test,Test.*,WindowsHack -bundle EverCrypt.Hash+EverCrypt.Hash.Incremental=[rename=EverCrypt_Hash] -library EverCrypt.AutoConfig,EverCrypt.OpenSSL,EverCrypt.BCrypt -minimal -add-include "kremlin/internal/types.h" -add-include "kremlin/internal/target.h" -add-include "kremlin/lowstar_endianness.h" -add-include <string.h> -fbuiltin-uint128 -bundle EverCrypt.AutoConfig2= -bundle EverCrypt -bundle EverCrypt.Hacl -bundle \*[rename=EverCrypt_Misc] -tmpdir dist/ccf/ -skip-compilation obj/prims.krml obj/FStar_Pervasives_Native.krml obj/FStar_Pervasives.krml obj/FStar_Float.krml obj/FStar_Mul.krml obj/FStar_Preorder.krml obj/FStar_Calc.krml obj/FStar_Squash.krml obj/FStar_Classical.krml obj/FStar_StrongExcludedMiddle.krml obj/FStar_FunctionalExtensionality.krml obj/FStar_List_Tot_Base.krml obj/FStar_List_Tot_Properties.krml obj/FStar_List_Tot.krml obj/FStar_Seq_Base.krml obj/FStar_Seq_Properties.krml obj/FStar_Seq.krml obj/FStar_Math_Lib.krml obj/FStar_Math_Lemmas.krml obj/FStar_BitVector.krml obj/FStar_UInt.krml obj/FStar_UInt32.krml obj/FStar_UInt64.krml obj/FStar_UInt8.krml obj/FStar_Exn.krml obj/FStar_Set.krml obj/FStar_Monotonic_Witnessed.krml obj/FStar_Ghost.krml obj/FStar_ErasedLogic.krml obj/FStar_PropositionalExtensionality.krml obj/FStar_PredicateExtensionality.krml obj/FStar_TSet.krml obj/FStar_Monotonic_Heap.krml obj/FStar_Heap.krml obj/FStar_ST.krml obj/FStar_All.krml obj/FStar_IO.krml obj/FStar_Map.krml obj/Vale_Lib_Seqs_s.krml obj/Vale_Def_Words_s.krml obj/Vale_Def_Words_Four_s.krml obj/Vale_Def_Words_Two_s.krml obj/Vale_Def_Words_Seq_s.krml obj/Vale_Def_Opaque_s.krml obj/Vale_Def_Types_s.krml obj/Vale_X64_Machine_s.krml obj/Vale_Lib_Map16.krml obj/Vale_Def_Prop_s.krml obj/Vale_X64_Xmms.krml obj/Vale_X64_Regs.krml obj/Vale_X64_Instruction_s.krml obj/Vale_Lib_Meta.krml obj/Vale_Def_Words_Two.krml obj/Vale_Lib_Seqs.krml obj/Vale_Def_TypesNative_s.krml obj/Vale_Arch_TypesNative.krml obj/Vale_Def_Words_Seq.krml obj/Vale_Arch_Types.krml obj/FStar_Option.krml obj/Vale_Lib_Set.krml obj/Vale_X64_Bytes_Code_s.krml obj/Vale_AES_AES_s.krml obj/Vale_Math_Poly2_Defs_s.krml obj/Vale_Math_Poly2_s.krml obj/Vale_Math_Poly2_Bits_s.krml obj/FStar_Monotonic_HyperHeap.krml obj/FStar_Monotonic_HyperStack.krml obj/FStar_HyperStack.krml obj/FStar_HyperStack_ST.krml obj/FStar_HyperStack_All.krml obj/FStar_Kremlin_Endianness.krml obj/FStar_Int.krml obj/FStar_Int64.krml obj/FStar_Int63.krml obj/FStar_Int32.krml obj/FStar_Int16.krml obj/FStar_Int8.krml obj/FStar_UInt63.krml obj/FStar_UInt16.krml obj/FStar_Int_Cast.krml obj/FStar_UInt128.krml obj/Spec_Hash_Definitions.krml obj/Spec_Hash_Lemmas0.krml obj/Spec_Hash_PadFinish.krml obj/Spec_Loops.krml obj/FStar_List.krml obj/Spec_SHA2_Constants.krml obj/Spec_SHA2.krml obj/Vale_X64_CryptoInstructions_s.krml obj/Vale_X64_CPU_Features_s.krml obj/Vale_X64_Instructions_s.krml obj/Vale_X64_Machine_Semantics_s.krml obj/Vale_X64_Bytes_Semantics.krml obj/FStar_Universe.krml obj/FStar_GSet.krml obj/FStar_ModifiesGen.krml obj/FStar_Range.krml obj/FStar_Reflection_Types.krml obj/FStar_Tactics_Types.krml obj/FStar_Tactics_Result.krml obj/FStar_Tactics_Effect.krml obj/FStar_Tactics_Util.krml obj/FStar_Reflection_Data.krml obj/FStar_Reflection_Const.krml obj/FStar_Char.krml obj/FStar_String.krml obj/FStar_Order.krml obj/FStar_Reflection_Basic.krml obj/FStar_Reflection_Derived.krml obj/FStar_Tactics_Builtins.krml obj/FStar_Reflection_Formula.krml obj/FStar_Reflection_Derived_Lemmas.krml obj/FStar_Reflection.krml obj/FStar_Tactics_Derived.krml obj/FStar_Tactics_Logic.krml obj/FStar_Tactics.krml obj/FStar_BigOps.krml obj/LowStar_Monotonic_Buffer.krml obj/LowStar_BufferView_Down.krml obj/LowStar_BufferView_Up.krml obj/Vale_Interop_Views.krml obj/LowStar_Buffer.krml obj/LowStar_Modifies.krml obj/LowStar_ModifiesPat.krml obj/LowStar_BufferView.krml obj/Vale_Lib_BufferViewHelpers.krml obj/LowStar_ImmutableBuffer.krml obj/Vale_Interop_Types.krml obj/Vale_Interop_Base.krml obj/Vale_Interop.krml obj/Vale_X64_Memory.krml obj/Vale_X64_Stack_i.krml obj/Vale_X64_Stack_Sems.krml obj/Vale_X64_BufferViewStore.krml obj/Vale_X64_Memory_Sems.krml obj/Vale_X64_State.krml obj/Vale_X64_StateLemmas.krml obj/Vale_X64_Lemmas.krml obj/Vale_X64_Print_s.krml obj/Vale_X64_Decls.krml obj/Vale_X64_Taint_Semantics.krml obj/Vale_X64_InsLemmas.krml obj/Vale_X64_QuickCode.krml obj/Vale_X64_InsAes.krml obj/Vale_Curve25519_Fast_lemmas_internal.krml obj/Vale_Curve25519_Fast_defs.krml obj/FStar_Tactics_CanonCommSwaps.krml obj/FStar_Algebra_CommMonoid.krml obj/FStar_Tactics_CanonCommMonoid.krml obj/FStar_Tactics_CanonCommSemiring.krml obj/Vale_Curve25519_FastUtil_helpers.krml obj/Vale_Curve25519_FastHybrid_helpers.krml obj/Vale_Curve25519_Fast_lemmas_external.krml obj/Vale_X64_QuickCodes.krml obj/Vale_X64_InsBasic.krml obj/Vale_X64_InsMem.krml obj/Vale_X64_InsVector.krml obj/Vale_X64_InsStack.krml obj/Vale_Curve25519_X64_FastHybrid.krml obj/Vale_Curve25519_FastSqr_helpers.krml obj/Vale_Curve25519_X64_FastSqr.krml obj/Vale_Curve25519_FastMul_helpers.krml obj/Vale_Curve25519_X64_FastMul.krml obj/Vale_Curve25519_X64_FastWide.krml obj/Vale_Curve25519_X64_FastUtil.krml obj/Vale_X64_MemoryAdapters.krml obj/Vale_Interop_Assumptions.krml obj/Vale_Interop_X64.krml obj/Vale_AsLowStar_ValeSig.krml obj/Vale_AsLowStar_LowStarSig.krml obj/Vale_AsLowStar_MemoryHelpers.krml obj/Vale_AsLowStar_Wrapper.krml obj/Vale_Stdcalls_X64_Fadd.krml obj/Vale_Wrapper_X64_Fadd.krml obj/Vale_Math_Poly2_Defs.krml obj/Vale_Math_Poly2.krml obj/Vale_Math_Poly2_Lemmas.krml obj/Vale_Math_Poly2_Bits.krml obj/Vale_Math_Poly2_Words.krml obj/Vale_AES_GF128_s.krml obj/Vale_AES_GF128.krml obj/Vale_AES_OptPublic.krml obj/Vale_AES_X64_GF128_Mul.krml obj/Vale_AES_X64_PolyOps.krml obj/Vale_X64_Stack.krml obj/Vale_AES_GCTR_s.krml obj/Vale_Lib_Workarounds.krml obj/Vale_AES_GCM_helpers.krml obj/Vale_AES_GCTR.krml obj/Vale_AES_AES256_helpers.krml obj/Vale_AES_X64_AES256.krml obj/Vale_AES_AES_helpers.krml obj/Vale_AES_X64_AES128.krml obj/Vale_AES_X64_AES.krml obj/FStar_BV.krml obj/Vale_Lib_Bv_s.krml obj/Vale_Math_Bits.krml obj/Vale_AES_GHash_s.krml obj/Vale_AES_GHash.krml obj/Vale_AES_X64_GF128_Init.krml obj/Vale_Lib_Operator.krml obj/Lib_LoopCombinators.krml obj/FStar_Int_Cast_Full.krml obj/Lib_IntTypes.krml obj/Lib_Sequence.krml obj/Spec_SHA3_Constants.krml obj/Spec_SHA1.krml obj/Spec_MD5.krml obj/Spec_Hash.krml obj/Spec_Hash_Incremental.krml obj/Spec_Hash_Lemmas.krml obj/LowStar_BufferOps.krml obj/C_Loops.krml obj/C_Endianness.krml obj/Hacl_Hash_Lemmas.krml obj/Hacl_Hash_Definitions.krml obj/Hacl_Hash_PadFinish.krml obj/Hacl_Hash_MD.krml obj/Spec_SHA2_Lemmas.krml obj/Vale_SHA_SHA_helpers.krml obj/Vale_X64_InsSha.krml obj/Vale_SHA_X64.krml obj/Vale_Stdcalls_X64_Sha.krml obj/Vale_SHA_Simplify_Sha.krml obj/Vale_Wrapper_X64_Sha.krml obj/Hacl_Hash_Core_SHA2_Constants.krml obj/Hacl_Hash_Core_SHA2.krml obj/Lib_RawIntTypes.krml obj/Lib_Loops.krml obj/Lib_ByteSequence.krml obj/Lib_Buffer.krml obj/FStar_Reflection_Arith.krml obj/FStar_Tactics_BV.krml obj/Vale_Lib_Tactics.krml obj/Vale_Poly1305_Bitvectors.krml obj/Vale_Math_Lemmas_Int.krml obj/FStar_Tactics_Canon.krml obj/Vale_Poly1305_Spec_s.krml obj/Vale_Poly1305_Math.krml obj/Vale_Poly1305_Util.krml obj/Vale_Poly1305_X64.krml obj/Vale_Stdcalls_X64_Poly.krml obj/Vale_Wrapper_X64_Poly.krml obj/FStar_Endianness.krml obj/Vale_Arch_BufferFriend.krml obj/Hacl_Hash_SHA2.krml obj/Hacl_Hash_Core_SHA1.krml obj/Hacl_Hash_SHA1.krml obj/Hacl_Hash_Core_MD5.krml obj/Hacl_Hash_MD5.krml obj/C.krml obj/C_String.krml obj/C_Failure.krml obj/FStar_Int128.krml obj/FStar_Int31.krml obj/FStar_UInt31.krml obj/FStar_Integers.krml obj/EverCrypt_StaticConfig.krml obj/Vale_Lib_X64_Cpuid.krml obj/Vale_Lib_X64_Cpuidstdcall.krml obj/Vale_Stdcalls_X64_Cpuid.krml obj/Vale_Wrapper_X64_Cpuid.krml obj/EverCrypt_TargetConfig.krml obj/EverCrypt_AutoConfig2.krml obj/EverCrypt_Helpers.krml obj/EverCrypt_Hash.krml obj/Hacl_Impl_Curve25519_Lemmas.krml obj/Spec_Curve25519_Lemmas.krml obj/Spec_Curve25519.krml obj/Hacl_Spec_Curve25519_Field51_Definition.krml obj/Hacl_Spec_Curve25519_Field51_Lemmas.krml obj/Hacl_Spec_Curve25519_Field51.krml obj/Hacl_Impl_Curve25519_Field51.krml obj/Hacl_Spec_Curve25519_Finv.krml obj/Hacl_Spec_Curve25519_Field64_Definition.krml obj/Hacl_Spec_Curve25519_Field64_Lemmas.krml obj/Hacl_Spec_Curve25519_Field64_Core.krml obj/Hacl_Spec_Curve25519_Field64.krml obj/Vale_Stdcalls_X64_Fswap.krml obj/Vale_Wrapper_X64_Fswap.krml obj/Vale_X64_Print_Inline_s.krml obj/Vale_Inline_X64_Fswap_inline.krml obj/Vale_Stdcalls_X64_Fsqr.krml obj/Vale_Wrapper_X64_Fsqr.krml obj/Vale_Inline_X64_Fsqr_inline.krml obj/Vale_Stdcalls_X64_Fmul.krml obj/Vale_Wrapper_X64_Fmul.krml obj/Vale_Inline_X64_Fmul_inline.krml obj/Vale_Stdcalls_X64_Fsub.krml obj/Vale_Wrapper_X64_Fsub.krml obj/Vale_Inline_X64_Fadd_inline.krml obj/Hacl_Impl_Curve25519_Field64_Core.krml obj/Hacl_Impl_Curve25519_Field64.krml obj/Hacl_Impl_Curve25519_Fields.krml obj/Lib_ByteBuffer.krml obj/Hacl_Impl_Curve25519_Finv.krml obj/Hacl_Curve25519_Finv_Field51.krml obj/Hacl_Bignum25519.krml obj/Hacl_Impl_Ed25519_PointAdd.krml obj/Hacl_Impl_Ed25519_PointDouble.krml obj/Hacl_Impl_Ed25519_SwapConditional.krml obj/Hacl_Impl_Ed25519_Ladder.krml obj/Hacl_Impl_Ed25519_PointCompress.krml obj/Hacl_Impl_Ed25519_SecretExpand.krml obj/Hacl_Impl_Ed25519_SecretToPublic.krml obj/Hacl_Impl_BignumQ_Mul.krml obj/Hacl_Impl_Load56.krml obj/Hacl_Impl_Store56.krml obj/Hacl_Impl_SHA512_ModQ.krml obj/Hacl_Impl_Ed25519_Sign_Steps.krml obj/Hacl_Impl_Ed25519_Sign.krml obj/Hacl_Impl_Ed25519_Sign_Expanded.krml obj/LowStar_Vector.krml obj/LowStar_Regional.krml obj/LowStar_RVector.krml obj/Vale_AES_GCM_s.krml obj/Vale_AES_GCM.krml obj/Hacl_Spec_Curve25519_AddAndDouble.krml obj/Hacl_Impl_Curve25519_AddAndDouble.krml obj/Lib_IntVector_Intrinsics.krml obj/Lib_IntVector.krml obj/Vale_AES_Gcm_simplify.krml obj/Vale_AES_X64_GHash.krml obj/Vale_AES_X64_AESCTR.krml obj/Vale_AES_X64_AESCTRplain.krml obj/Vale_AES_X64_GCTR.krml obj/Vale_AES_X64_GCMencrypt.krml obj/Vale_AES_X64_GCMdecrypt.krml obj/Vale_Stdcalls_X64_GCMdecrypt.krml obj/Vale_Stdcalls_X64_Aes.krml obj/Vale_Wrapper_X64_AES.krml obj/Vale_Wrapper_X64_GCMdecrypt.krml obj/Spec_Chacha20.krml obj/Hacl_Impl_Chacha20_Core32.krml obj/Hacl_Impl_Chacha20.krml obj/Hacl_Spec_Poly1305_Vec.krml obj/Hacl_Impl_Poly1305_Lemmas.krml obj/Hacl_Spec_Poly1305_Field32xN.krml obj/Hacl_Poly1305_Field32xN_Lemmas.krml obj/Hacl_Spec_Poly1305_Field32xN_Lemmas.krml obj/Hacl_Impl_Poly1305_Field32xN.krml obj/Hacl_Impl_Poly1305_Fields.krml obj/Spec_Poly1305.krml obj/Hacl_Spec_Poly1305_Equiv_Lemmas.krml obj/Hacl_Spec_Poly1305_Equiv.krml obj/Hacl_Impl_Poly1305.krml obj/Hacl_Poly1305_32.krml obj/Spec_Chacha20Poly1305.krml obj/Hacl_Impl_Chacha20Poly1305_PolyCore.krml obj/Hacl_Impl_Chacha20Poly1305_Poly.krml obj/Hacl_Impl_Chacha20Poly1305.krml obj/FStar_Dyn.krml obj/EverCrypt_Vale.krml obj/EverCrypt_Specs.krml obj/EverCrypt_OpenSSL.krml obj/EverCrypt_Hacl.krml obj/EverCrypt_BCrypt.krml obj/EverCrypt_Cipher.krml obj/Hacl_Impl_Curve25519_Generic.krml obj/Hacl_Curve25519_51.krml obj/Hacl_Curve25519_64.krml obj/EverCrypt_Curve25519.krml obj/Hacl_Poly1305_128.krml obj/Hacl_Poly1305_256.krml obj/Vale_Poly1305_Equiv.krml obj/Vale_Poly1305_CallingFromLowStar.krml obj/EverCrypt_Poly1305.krml obj/Spec_HMAC.krml obj/Hacl_HMAC.krml obj/EverCrypt_HMAC.krml obj/Spec_HKDF.krml obj/EverCrypt_HKDF.krml obj/EverCrypt.krml obj/MerkleTree_Spec.krml obj/MerkleTree_New_High.krml obj/LowStar_Regional_Instances.krml obj/MerkleTree_New_Low.krml obj/MerkleTree_New_Low_Serialization.krml obj/Vale_AES_X64_AESopt2.krml obj/Vale_AES_X64_AESGCM.krml obj/Vale_AES_X64_GCMencryptOpt.krml obj/Vale_AES_X64_GCMdecryptOpt.krml obj/Vale_Stdcalls_X64_GCMdecryptOpt.krml obj/Vale_Wrapper_X64_GCMdecryptOpt.krml obj/Vale_Wrapper_X64_GCMdecryptOpt256.krml obj/Vale_Stdcalls_X64_GCMencryptOpt.krml obj/Vale_Wrapper_X64_GCMencryptOpt.krml obj/Vale_Wrapper_X64_GCMencryptOpt256.krml obj/Vale_Stdcalls_X64_AesHash.krml obj/Vale_Wrapper_X64_AEShash.krml obj/Spec_AEAD.krml obj/EverCrypt_Error.krml obj/EverCrypt_AEAD.krml obj/WasmSupport.krml obj/MerkleTree_New_High_Correct_Base.krml obj/MerkleTree_New_High_Correct_Rhs.krml obj/MerkleTree_New_High_Correct_Path.krml obj/MerkleTree_New_High_Correct_Flushing.krml obj/MerkleTree_New_High_Correct_Insertion.krml obj/MerkleTree_New_High_Correct.krml obj/EverCrypt_Hash_Incremental.krml obj/Test_Hash.krml obj/Spec_SHA3.krml obj/Hacl_Impl_SHA3.krml obj/TestLib.krml obj/EverCrypt_Chacha20Poly1305.krml obj/Hacl_SHA3.krml obj/Lib_PrintBuffer.krml obj/Hacl_Test_CSHAKE.krml obj/Spec_Hash_Test.krml obj/Hacl_Impl_Ed25519_Pow2_252m2.krml obj/Hacl_Impl_Ed25519_RecoverX.krml obj/Hacl_Impl_Ed25519_PointDecompress.krml obj/Hacl_Impl_Ed25519_PointEqual.krml obj/Hacl_Impl_Ed25519_Verify.krml obj/Hacl_Ed25519.krml obj/Hacl_Test_Ed25519.krml obj/Vale_Stdcalls_X64_GCTR.krml obj/Vale_X64_Leakage_s.krml obj/Vale_Wrapper_X64_GCTR.krml obj/Test_Vectors_Chacha20Poly1305.krml obj/Lib_RandomBuffer.krml obj/Test_Vectors_Aes128.krml obj/Vale_Test_X64_Args.krml obj/Vale_Stdcalls_X64_GCMencrypt.krml obj/LowStar_Endianness.krml obj/Hacl_Hash_Agile.krml obj/Vale_Test_X64_Vale_memcpy.krml obj/Hacl_Test_SHA3.krml obj/Test_Vectors_Curve25519.krml obj/Vale_Test_X64_Memcpy.krml obj/Spec_Chacha20Poly1305_Test.krml obj/Vale_Lib_Lists.krml obj/Vale_Wrapper_X64_GCMencrypt.krml obj/Vale_AES_X64_AESopt.krml obj/Test_Vectors_Poly1305.krml obj/Vale_AsLowStar_Test.krml obj/Vale_X64_Leakage_Helpers.krml obj/Vale_X64_Leakage_Ins.krml obj/Vale_X64_Leakage.krml obj/Spec_SHA3_Test.krml obj/Test_Lowstarize.krml obj/Test_Vectors.krml obj/Test_NoHeap.krml obj/Test_Vectors_Aes128Gcm.krml obj/Spec_Curve25519_Test.krml obj/Test.krml obj/Spec_Chacha20_Test.krml -silent -ccopt -Wno-unused -warn-error @4-6 -fparentheses Hacl_AES.c Lib_RandomBuffer.c Lib_PrintBuffer.c evercrypt_vale_stubs.c -o libevercrypt.a
  F* version: 7c70b890
  KreMLin version: b511d90c
 */

#include "Hacl_Chacha20Poly1305.h"

static void
Hacl_Impl_Chacha20Poly1305_PolyCore_poly1305_padded(
  uint64_t *ctx,
  uint32_t len1,
  uint8_t *text,
  uint8_t *tmp
)
{
  uint32_t n1 = len1 / (uint32_t)16U;
  uint32_t r = len1 % (uint32_t)16U;
  uint8_t *blocks = text;
  uint8_t *rem1 = text + n1 * (uint32_t)16U;
  Hacl_Poly1305_32_poly1305_update_padded(ctx, n1 * (uint32_t)16U, blocks);
  memcpy(tmp, rem1, r * sizeof rem1[0U]);
  if (r > (uint32_t)0U)
  {
    Hacl_Poly1305_32_poly1305_update_padded(ctx, (uint32_t)16U, tmp);
  }
}

static void Hacl_Impl_Chacha20Poly1305_PolyCore_poly1305_init(uint64_t *ctx, uint8_t *k)
{
  Hacl_Poly1305_32_poly1305_init(ctx, k);
}

static void
Hacl_Impl_Chacha20Poly1305_PolyCore_update1(uint64_t *ctx, uint32_t len1, uint8_t *text)
{
  Hacl_Poly1305_32_poly1305_update_padded(ctx, len1, text);
}

static void Hacl_Impl_Chacha20Poly1305_PolyCore_finish(uint64_t *ctx, uint8_t *k, uint8_t *out)
{
  Hacl_Poly1305_32_poly1305_finish(out, k, ctx);
}

static void Hacl_Impl_Chacha20Poly1305_Poly_derive_key(uint8_t *k, uint8_t *n1, uint8_t *out)
{
  uint32_t ctx[16U] = { 0U };
  uint32_t ctx_core[16U] = { 0U };
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
  ctx[12U] = (uint32_t)0U;
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
  Hacl_Impl_Chacha20_chacha20_core(ctx_core, ctx, (uint32_t)0U);
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    store32_le(out + i * (uint32_t)4U, ctx_core[i]);
  }
}

static void
Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_padded(
  uint32_t aadlen,
  uint8_t *aad,
  uint32_t mlen,
  uint8_t *m,
  uint64_t *ctx
)
{
  uint8_t block[16U] = { 0U };
  Hacl_Impl_Chacha20Poly1305_PolyCore_poly1305_padded(ctx, aadlen, aad, block);
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint8_t *os = block;
    uint8_t x = (uint8_t)0U;
    os[i] = x;
  }
  Hacl_Impl_Chacha20Poly1305_PolyCore_poly1305_padded(ctx, mlen, m, block);
}

static void
Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_to_bytes(
  uint32_t aadlen,
  uint32_t mlen,
  uint8_t *block
)
{
  uint8_t *aad_len8 = block;
  store64_le(aad_len8, (uint64_t)aadlen);
  uint8_t *cipher_len8 = block + (uint32_t)8U;
  store64_le(cipher_len8, (uint64_t)mlen);
}

static void
Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_finish(
  uint8_t *k,
  uint8_t *out,
  uint64_t *ctx,
  uint8_t *block
)
{
  Hacl_Impl_Chacha20Poly1305_PolyCore_update1(ctx, (uint32_t)16U, block);
  Hacl_Impl_Chacha20Poly1305_PolyCore_finish(ctx, k, out);
}

static void
Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_(
  uint8_t *k,
  uint32_t aadlen,
  uint8_t *aad,
  uint32_t mlen,
  uint8_t *m,
  uint8_t *out,
  uint64_t *ctx,
  uint8_t *block
)
{
  Hacl_Impl_Chacha20Poly1305_PolyCore_poly1305_init(ctx, k);
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_padded(aadlen, aad, mlen, m, ctx);
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_to_bytes(aadlen, mlen, block);
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_finish(k, out, ctx, block);
}

static void
Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core(
  uint8_t *k,
  uint32_t aadlen,
  uint8_t *aad,
  uint32_t mlen,
  uint8_t *m,
  uint8_t *out
)
{
  KRML_CHECK_SIZE(sizeof (uint64_t), (uint32_t)5U + (uint32_t)20U);
  uint64_t ctx[(uint32_t)5U + (uint32_t)20U];
  memset(ctx, 0U, ((uint32_t)5U + (uint32_t)20U) * sizeof ctx[0U]);
  uint8_t block[16U] = { 0U };
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core_(k, aadlen, aad, mlen, m, out, ctx, block);
}

static void
Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do(
  uint8_t *k,
  uint8_t *n1,
  uint32_t aadlen,
  uint8_t *aad,
  uint32_t mlen,
  uint8_t *m,
  uint8_t *out
)
{
  uint8_t tmp[64U] = { 0U };
  Hacl_Impl_Chacha20Poly1305_Poly_derive_key(k, n1, tmp);
  uint8_t *key = tmp;
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do_core(key, aadlen, aad, mlen, m, out);
}

void
Hacl_Impl_Chacha20Poly1305_aead_encrypt_chacha_poly(
  uint8_t *k,
  uint8_t *n1,
  uint32_t aadlen,
  uint8_t *aad,
  uint32_t mlen,
  uint8_t *m,
  uint8_t *cipher,
  uint8_t *mac
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
    uint8_t *bj = k + i * (uint32_t)4U;
    uint32_t u = load32_le(bj);
    uint32_t r = u;
    uint32_t x = r;
    os[i] = x;
  }
  ctx[12U] = (uint32_t)1U;
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
  uint32_t k1[16U] = { 0U };
  uint32_t rem1 = mlen % (uint32_t)64U;
  uint32_t nb = mlen / (uint32_t)64U;
  uint32_t rem2 = mlen % (uint32_t)64U;
  for (uint32_t i0 = (uint32_t)0U; i0 < nb; i0 = i0 + (uint32_t)1U)
  {
    uint8_t *uu____3 = cipher + i0 * (uint32_t)64U;
    uint8_t *uu____4 = m + i0 * (uint32_t)64U;
    uint32_t k2[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k2, ctx, i0);
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
      uint32_t x = bl[i] ^ k2[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(uu____3 + i * (uint32_t)4U, bl[i]);
    }
  }
  if (rem2 > (uint32_t)0U)
  {
    uint8_t *uu____5 = cipher + nb * (uint32_t)64U;
    uint8_t *uu____6 = m + nb * (uint32_t)64U;
    uint8_t plain[64U] = { 0U };
    memcpy(plain, uu____6, rem1 * sizeof uu____6[0U]);
    uint32_t k2[16U] = { 0U };
    Hacl_Impl_Chacha20_chacha20_core(k2, ctx, nb);
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
      uint32_t x = bl[i] ^ k2[i];
      os[i] = x;
    }
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
    {
      store32_le(plain + i * (uint32_t)4U, bl[i]);
    }
    memcpy(uu____5, plain, rem1 * sizeof plain[0U]);
  }
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do(k, n1, aadlen, aad, mlen, cipher, mac);
}

uint32_t
Hacl_Impl_Chacha20Poly1305_aead_decrypt_chacha_poly(
  uint8_t *k,
  uint8_t *n1,
  uint32_t aadlen,
  uint8_t *aad,
  uint32_t mlen,
  uint8_t *m,
  uint8_t *cipher,
  uint8_t *mac
)
{
  uint8_t computed_mac[16U] = { 0U };
  Hacl_Impl_Chacha20Poly1305_Poly_poly1305_do(k, n1, aadlen, aad, mlen, cipher, computed_mac);
  uint8_t res0 = (uint8_t)255U;
  for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
  {
    uint8_t uu____0 = FStar_UInt8_eq_mask(computed_mac[i], mac[i]);
    res0 = uu____0 & res0;
  }
  uint8_t z = res0;
  uint32_t res;
  if (z == (uint8_t)255U)
  {
    uint32_t ctx[16U] = { 0U };
    uint32_t *uu____1 = ctx;
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)4U; i = i + (uint32_t)1U)
    {
      uint32_t *os = uu____1;
      uint32_t x = Hacl_Impl_Chacha20_chacha20_constants[i];
      os[i] = x;
    }
    uint32_t *uu____2 = ctx + (uint32_t)4U;
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)8U; i = i + (uint32_t)1U)
    {
      uint32_t *os = uu____2;
      uint8_t *bj = k + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    ctx[12U] = (uint32_t)1U;
    uint32_t *uu____3 = ctx + (uint32_t)13U;
    for (uint32_t i = (uint32_t)0U; i < (uint32_t)3U; i = i + (uint32_t)1U)
    {
      uint32_t *os = uu____3;
      uint8_t *bj = n1 + i * (uint32_t)4U;
      uint32_t u = load32_le(bj);
      uint32_t r = u;
      uint32_t x = r;
      os[i] = x;
    }
    uint32_t k1[16U] = { 0U };
    uint32_t rem1 = mlen % (uint32_t)64U;
    uint32_t nb = mlen / (uint32_t)64U;
    uint32_t rem2 = mlen % (uint32_t)64U;
    for (uint32_t i0 = (uint32_t)0U; i0 < nb; i0 = i0 + (uint32_t)1U)
    {
      uint8_t *uu____4 = m + i0 * (uint32_t)64U;
      uint8_t *uu____5 = cipher + i0 * (uint32_t)64U;
      uint32_t k2[16U] = { 0U };
      Hacl_Impl_Chacha20_chacha20_core(k2, ctx, i0);
      uint32_t bl[16U] = { 0U };
      for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
      {
        uint32_t *os = bl;
        uint8_t *bj = uu____5 + i * (uint32_t)4U;
        uint32_t u = load32_le(bj);
        uint32_t r = u;
        uint32_t x = r;
        os[i] = x;
      }
      for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
      {
        uint32_t *os = bl;
        uint32_t x = bl[i] ^ k2[i];
        os[i] = x;
      }
      for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
      {
        store32_le(uu____4 + i * (uint32_t)4U, bl[i]);
      }
    }
    if (rem2 > (uint32_t)0U)
    {
      uint8_t *uu____6 = m + nb * (uint32_t)64U;
      uint8_t *uu____7 = cipher + nb * (uint32_t)64U;
      uint8_t plain[64U] = { 0U };
      memcpy(plain, uu____7, rem1 * sizeof uu____7[0U]);
      uint32_t k2[16U] = { 0U };
      Hacl_Impl_Chacha20_chacha20_core(k2, ctx, nb);
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
        uint32_t x = bl[i] ^ k2[i];
        os[i] = x;
      }
      for (uint32_t i = (uint32_t)0U; i < (uint32_t)16U; i = i + (uint32_t)1U)
      {
        store32_le(plain + i * (uint32_t)4U, bl[i]);
      }
      memcpy(uu____6, plain, rem1 * sizeof plain[0U]);
    }
    res = (uint32_t)0U;
  }
  else
  {
    res = (uint32_t)1U;
  }
  return res;
}


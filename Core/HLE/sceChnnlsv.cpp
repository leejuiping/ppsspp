// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include "HLE.h"

#include "sceChnnlsv.h"
#include "sceKernel.h"
extern "C"
{
#include "ext/libkirk/kirk_engine.h"
}

u8 dataBuf[2048+20];
u8* dataBuf2 = dataBuf + 20;

static const u8 hash198C[16] = {0xFA, 0xAA, 0x50, 0xEC, 0x2F, 0xDE, 0x54, 0x93, 0xAD, 0x14, 0xB2, 0xCE, 0xA5, 0x30, 0x05, 0xDF };
static const u8 hash19BC[16] = {0xCB, 0x15, 0xF4, 0x07, 0xF9, 0x6A, 0x52, 0x3C, 0x04, 0xB9, 0xB2, 0xEE, 0x5C, 0x53, 0xFA, 0x86 };

static const u8 key000019CC[16] = {0x70, 0x44, 0xA3, 0xAE, 0xEF, 0x5D, 0xA5, 0xF2, 0x85, 0x7F, 0xF2, 0xD6, 0x94, 0xF5, 0x36, 0x3B};
static const u8 key000019DC[16] = {0xEC, 0x6D, 0x29, 0x59, 0x26, 0x35, 0xA5, 0x7F, 0x97, 0x2A, 0x0D, 0xBC, 0xA3, 0x26, 0x33, 0x00};
static const u8 key0000199C[16] = {0x36, 0xA5, 0x3E, 0xAC, 0xC5, 0x26, 0x9E, 0xA3, 0x83, 0xD9, 0xEC, 0x25, 0x6C, 0x48, 0x48, 0x72};
static const u8 key000019AC[16] = {0xD8, 0xC0, 0xB0, 0xF3, 0x3E, 0x6B, 0x76, 0x85, 0xFD, 0xFB, 0x4D, 0x7D, 0x45, 0x1E, 0x92, 0x03};

int sub_000014BC(u8* data, int length)
{
	*(s32*)(data + 0) = 5;
	*(s32*)(data + 12) = 256;
	*(s32*)(data + 4) = 0;
	*(s32*)(data + 8) = 0;
	*(s32*)(data + 16) = length;
	int res = sceUtilsBufferCopyWithRange(data, length + 20, data, length + 20, 8);
	if (res == 0)
		return 0;
	return -258;
}

int sub_00001418(u8* data, int length, int val2)
{
	*(s32*)(data + 12) = val2;
	*(s32*)(data + 0) = 5;
	*(s32*)(data + 4) = 0;
	*(s32*)(data + 8) = 0;
	*(s32*)(data + 16) = length;
	int res = sceUtilsBufferCopyWithRange(data, length + 20, data, length + 20, 7);
	if (res == 0)
		return 0;
	return -257;
}


int sub_000015B0(u8* data, int alignedLen, u8* buf, int val)
{
	u8 sp0[16];

	for(int i = 0; i < 16; i++)
	{
		sp0[i] = data[alignedLen+i+4];
	}
	int res = sub_00001418(data, alignedLen, val);
	if (res != 0)
	{
		return res;
	}

	for(int i = 0; i < 16; i++)
	{
		data[i] = data[i] ^ buf[i];
	}
	for(int i = 0; i < 16; i++)
	{
		buf[i] = sp0[i];
	}
	return 0;
}

int sub_00000000(u8* data_out, u8* data, int alignedLen, u8* data2, int& data3, int mode)
{
	for(int i = 0; i < 16; i++)
	{
		data_out[20+i] = data2[i];
	}
	int valS4 = 100;
	int res;
	if (mode == 6)
	{
		valS4 = 100;
		for(int i = 0; i < 16; i++)
		{
			data_out[20+i] = data_out[20+i] ^ key000019DC[i];
		}
		res = sub_000014BC(data_out, 16);
		for(int i = 0; i < 16; i++)
		{
			data_out[i] = data_out[i] ^ key000019CC[i];
		}
	}
	else if (mode == 4)
	{
		valS4 = 87;
		for(int i = 0; i < 16; i++)
		{
			data_out[20+i] = data_out[20+i] ^ key000019AC[i];
		}
		res = sub_000014BC(data_out, 16);
		for(int i = 0; i < 16; i++)
		{
			data_out[i] = data_out[i] ^ key0000199C[i];
		}
	}
	else if (mode == 2)
	{
		res = sub_000014BC(data_out, 16);
		valS4 = 83;
	}
	else if (mode == 1)
	{
		res = sub_00001418(data_out, 16, 4);
		valS4 = 83;
	}
	else  if (mode == 3)
	{
		valS4 = 87;
		for(int i = 0; i < 16; i++)
		{
			data_out[i+20] = data_out[i+20] ^ key000019AC[i];
		}
		res = sub_00001418(data_out, 16, 14);
		for(int i = 0; i < 16; i++)
		{
			data_out[i] = data_out[i] ^ key0000199C[i];
		}
	}
	else
	{
		valS4 = 100;
		for(int i = 0; i < 16; i++)
		{
			data_out[i + 20] = data_out[i + 20] ^ key000019DC[i];
		}
		res = sub_00001418(data_out, 16, 18);
		for(int i = 0; i < 16; i++)
		{
			data_out[i] = data_out[i] ^ key000019CC[i];
		}
	}

	u8 sp16[16];
	if (res != 0)
	{
		return res;
	}
	for(int i = 0; i < 16; i++)
	{
		sp16[i] = data_out[i];
	}
	u8 sp0[16];
	if (data3 == 1)
	{
		for(int i = 0; i < 16; i++)
		{
			sp0[i] = 0;
		}
	}
	else
	{
		for(int i = 0; i < 12; i++)
		{
			sp0[i] = sp16[i];
		}
		sp0[12] = (data3-1)  & 0xFF ;
		sp0[13] = ((data3-1) >> 8) & 0xFF;
		sp0[14] = ((data3-1) >> 16) & 0xFF;
		sp0[15] = ((data3-1) >> 24) & 0xFF;
	}

	if ((u32)20 < (u32)alignedLen + 20)
	{
		for(int i = 20; i < alignedLen + 20; i += 16)
		{
			for(int j = 0; j < 12; j++)
			{
				data_out[i+j] = sp16[j];
			}
			data_out[12+i] = data3;
			data_out[13+i] = (data3 >> 8) & 0xFF;
			data_out[14+i] = (data3 >> 16) & 0xFF;
			data_out[15+i] = (data3 >> 24) & 0xFF;
			data3++;
		}
	}

	res = sub_000015B0(data_out, alignedLen, sp0, valS4);
	if (res != 0)
	{
		return res;
	}
	if (res >= alignedLen)
	{
		return 0;
	}
	for(int i = 0; i < alignedLen; i++)
	{
		data[i] = data[i] ^ data_out[i];
	}

	return 0;
}

int sub_000013C8(u8* data, int size, int num)
{
	*(int*)(data+0) = 4;
	*(int*)(data+4) = 0;
	*(int*)(data+8) = 0;
	*(int*)(data+12) = num;
	*(int*)(data+16) = size;
	size = size + 20;

	int res = sceUtilsBufferCopyWithRange(data,size,data,size,4);
	if(res != 0)
	{
		return -257;
	}
	return 0;
}

int sub_00001468(u8* data, int size)
{
	*(int*)(data+0) = 4;
	*(int*)(data+12) = 256;
	*(int*)(data+4) = 0;
	*(int*)(data+8) = 0;
	*(int*)(data+16) = size;
	size = size + 20;

	int res = sceUtilsBufferCopyWithRange(data,size,data,size,5);
	if(res != 0)
	{
		return -258;
	}
	return 0;
}

int sub_00001510(u8* data, int size, u8* result , int num)
{
	for(int i = 0; i < 16; i++)
	{
		int v1 = data[i+20];
		v1 = v1 ^ result[i];
		data[i+20] = v1;
	}

	int res = sub_000013C8(data, size,  num);
	if(res != 0)
	{
		return res;
	}

	for(int i = 0; i < 16; i++)
	{
		result[i] = data[size + i + 4];
	}
	return 0;
}

int sub_000017A8(u8* data)
	{
	int res = sceUtilsBufferCopyWithRange(data, 20, 0, 0, 14);
	if (res == 0)
		return 0;
	return -261;
}

int sceSdGetLastIndex(u32 addressCtx,u32 addressHash, u32 addressKey)
{
	pspChnnlsvContext1 ctx;
	Memory::ReadStruct(addressCtx,&ctx);
	u8* in_hash;
	u8* in_key;
	in_hash = Memory::GetPointer(addressHash);
	in_key = Memory::GetPointer(addressKey);

	int res = sceSdGetLastIndex_(ctx, in_hash, in_key);

	Memory::WriteStruct(addressCtx,&ctx);
	return res;
}
int sceSdGetLastIndex_(pspChnnlsvContext1& ctx, u8* in_hash, u8* in_key)
{
	if(ctx.keyLength >= 17)
	{
		return -1026;
	}
	int num = 0;
	switch(ctx.mode)
	{
	case 6:
		num = 17;
		break;
	case 4:
		num = 13;
		break;
	case 2:
		num = 5;
		break;
	case 1:
		num = 3;
		break;
	case 3:
		num = 12;
		break;
	default:
		num = 16;
		break;
	}

	memset(dataBuf2,0,16);

	int res = sub_000013C8(dataBuf,16,num);
	if(res != 0)
	{
		return res;
	}

	u8 data1[16];
	u8 data2[16];

	memcpy(data1,dataBuf2,16);
	int tmp1 = 0;
	if((s8)data1[0] < 0)
		tmp1 = 135;

	for(int i = 0; i < 15; i++)
	{
		u8 val1 = data1[i] << 1;
		u8 val2 = data1[i+1] >> 7;
		data1[i] = val1 | val2;
	}

	u8 tmp2 = data1[15] << 1;
	tmp2 = tmp1 ^ tmp2;
	data1[15] = tmp2;

	if(ctx.keyLength < 16)
	{
		tmp1 = 0;
		if((s8)data1[0] < 0)
		{
			tmp1 = 135;
		}
		for(int i = 0; i < 15; i++)
		{
			u8 val1 = data1[i] << 1;
			u8 val2 = data1[i+1] >> 7;
			data1[i] = val1 | val2;
		}
		u8 tmp2 = data1[15] << 1;
		tmp2 = tmp1 ^ tmp2;
		data1[15] = tmp2;

		int oldKeyLength = ctx.keyLength;
		*(s8*)(ctx.key + ctx.keyLength) = -128;
		if(oldKeyLength + 1 < 16)
		{
			for(int i = oldKeyLength + 1; i < 16; i++)
			{
				*(s8*)(ctx.key + i) = 0;
			}
		}
	}

	for(int i = 0; i < 16; i++)
	{
		ctx.key[i] = ctx.key[i] ^ data1[i];
	}

	for(int i = 0; i < 16; i++)
	{
		dataBuf2[i] = ctx.key[i];
	}

	for(int i = 0; i < 16; i++)
	{
		data2[i] = ctx.result[i];
	}
	int ret = sub_00001510(dataBuf,16,data2,num);
	if(ret != 0)
	{
		return ret;
	}

	if((u32)(ctx.mode-3) < 2)
	{
		for(int i = 0; i < 16; i++)
		{
			data2[i] = data2[i] ^ hash198C[i];
		}
	}
	else if((u32)(ctx.mode-5) < 2)
	{
		for(int i = 0; i < 16; i++)
		{
			data2[i] = data2[i] ^ hash19BC[i];
		}
	}

	int cond = ((ctx.mode ^ 0x2) < 1 || (ctx.mode ^ 0x4) < 1 || ctx.mode == 6);
	if(cond != 0)
	{
		for(int i = 0; i < 16; i++)
		{
			dataBuf2[i] = data2[i];
		}
		int ret = sub_00001468(dataBuf,16);
		if(ret != 0)
		{
			return ret;
		}
		int res = sub_000013C8(dataBuf,16,num);
		if(res != 0)
		{
			return res;
		}

		for(int i = 0; i < 16; i++)
		{
			data2[i] = dataBuf2[i];
		}
	}

	if(in_key != 0)
	{
		for(int i = 0; i < 16; i++)
		{
			data2[i] = in_key[i] ^ data2[i];
		}

		for(int i = 0; i < 16; i++)
		{
			dataBuf2[i] = data2[i];
		}

		int res = sub_000013C8(dataBuf,16,num);
		if(res != 0)
		{
			return res;
		}
		for(int i = 0; i < 16; i++)
		{
			data2[i] = dataBuf2[i];
		}
	}
	for(int i = 0; i < 16; i++)
	{
		in_hash[i] = data2[i];
	}

	for(int i = 0; i < 16; i++)
	{
		ctx.result[i] = 0;
	}

	for(int i = 0; i < 16; i++)
	{
		ctx.key[i] = 0;
	}
	ctx.keyLength = 0;
	ctx.mode = 0;

	return 0;
}

int sceSdSetIndex(u32 addressCtx, int value)
{
	pspChnnlsvContext1 ctx;
	Memory::ReadStruct(addressCtx,&ctx);
	int res = sceSdSetIndex_(ctx, value);
	Memory::WriteStruct(addressCtx,&ctx);
	return res;
}

int sceSdSetIndex_(pspChnnlsvContext1& ctx, int value)
{
	ctx.mode = value;
	memset(ctx.result,0,16);
	memset(ctx.key,0,16);
	ctx.keyLength = 0;
	return 0;
}


int sceSdRemoveValue(u32 addressCtx, u32 addressData, int length)
{
	pspChnnlsvContext1 ctx;
	Memory::ReadStruct(addressCtx,&ctx);
	u8* data;
	data = Memory::GetPointer(addressData);

	int res = sceSdRemoveValue_(ctx, data, length);

	Memory::WriteStruct(addressCtx,&ctx);
	return res;
}

int sceSdRemoveValue_(pspChnnlsvContext1& ctx, u8* data, int length)
{
	if(ctx.keyLength >= 17)
	{
		return -1026;
	}
	if(ctx.keyLength + length < 17)
	{
		if(length == 0)
		{
			return 0;
		}
		for(int i = 0; i < length; i++)
		{
			ctx.key[ctx.keyLength+i] = data[i];
		}
		ctx.keyLength = ctx.keyLength + length;
		return 0;
	}
	int mode = ctx.mode;
	int num = 0;
	switch(mode)
	{
	case 6:
		num = 17;
		break;
	case 4:
		num = 13;
		break;
	case 2:
		num = 5;
		break;
	case 1:
		num = 3;
		break;
	case 3:
		num = 12;
		break;
	default:
		num = 16;
		break;
	}

	memset(dataBuf2,0,2048);

	if(ctx.keyLength > 0)
	{
		memcpy(dataBuf2,ctx.key,ctx.keyLength);
	}
	int len = (ctx.keyLength + length) & 0xF;
	if(len == 0) len = 16;

	int oldLength = ctx.keyLength;
	ctx.keyLength = len;

	int diff = length - len;
	if(len != 0)
	{
		memcpy(ctx.key,data+diff,len);
	}

	int newSize = oldLength;
	if(diff != 0)
	{
		for(int i = 0; i < diff; i++)
		{
			if(newSize == 2048)
			{
				int res = sub_00001510(dataBuf,2048,ctx.result,num);
				if(res != 0)
				{
					return res;
				}
				newSize = 0;
			}
			dataBuf2[newSize] = data[i];
			newSize++;
		}
	}
	if(newSize == 0)
	{
		return 0;
	}
	int res = sub_00001510(dataBuf,newSize,ctx.result, num);
	if(res == 0)
	{
		return res;
	}
	return 0;
}

int sceSdCreateList(u32 ctx2Addr, int mode, int unkwn, u32 dataAddr, u32 cryptkeyAddr)
{
	pspChnnlsvContext2 ctx2;
	Memory::ReadStruct(ctx2Addr, &ctx2);
	u8* data = Memory::GetPointer(dataAddr);
	u8* cryptkey = Memory::GetPointer(cryptkeyAddr);

	int res = sceSdCreateList_(ctx2, mode, unkwn, data, cryptkey);

	Memory::WriteStruct(ctx2Addr, &ctx2);

	return res;
}

int sceSdCreateList_(pspChnnlsvContext2& ctx2, int mode, int uknw, u8* data, u8* cryptkey)
{
	ctx2.mode = mode;
	ctx2.unkn = 1;
	if (uknw == 2)
	{
		for(int i = 0; i < 16; i++)
		{
			ctx2.unknown[i] = data[i];
		}
		if (cryptkey == 0)
		{
			return 0;
		}

		for(int i = 0; i < 16; i++)
		{
			ctx2.unknown[i] = ctx2.unknown[i] ^ cryptkey[i];
		}
		return 0;
	}
	else if (uknw == 1)
	{
		u8 kirkHeader[37];
		u8* kirkData = kirkHeader+20;
		int res = sub_000017A8(kirkHeader);
		if (res != 0)
		{
			return res;
		}
		for(int i = 15; i >= 0 ; i--)
		{
			kirkHeader[i+20] = kirkHeader[i];
		}
		for(int i = 0; i < 4; i++)
		{
			kirkHeader[i+32] = 0;
		}

		if (mode == 6)
		{
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key000019CC[i];
			}
			res = sub_00001468(kirkHeader, 16);
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key000019DC[i];
			}
		}
		else if (mode == 4)
		{
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key0000199C[i];
			}
			res = sub_00001468(kirkHeader, 16);
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key000019AC[i];
			}
		}
		else if (mode == 2)
		{
			res = sub_00001468(kirkHeader, 16);
		}
		else if (mode == 1)
		{
			res = sub_000013C8(kirkHeader, 16, 4);
		}
		else if (mode == 3)
		{
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key0000199C[i];
			}
			res = sub_000013C8(kirkHeader, 16, 14);
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] =  kirkData[i] ^ key000019AC[i];
			}
		}
		else
		{
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key000019CC[i];
			}
			res = sub_000013C8(kirkHeader, 16, 18);
			for(int i = 0; i < 16; i++)
			{
				kirkData[i] = kirkData[i] ^ key000019DC[i];
			}
		}

		if (res != 0)
		{
			return res;
		}

		for(int i = 0; i < 16; i++)
		{
			ctx2.unknown[i] = kirkData[i];
		}
		for(int i = 0; i < 16; i++)
		{
			data[i] = kirkData[i];
		}
		if (cryptkey != 0)
		{
			for(int i = 0; i < 16; i++)
			{
				ctx2.unknown[i] = ctx2.unknown[i] ^ cryptkey[i];
			}
		}
	}

	return 0;
}

int sceSdSetMember(u32 ctxAddr, u32 dataAddr, int alignedLen)
{
	pspChnnlsvContext2 ctx;
	Memory::ReadStruct(ctxAddr, &ctx);
	u8* data = Memory::GetPointer(dataAddr);

	int res = sceSdSetMember_(ctx, data, alignedLen);

	Memory::WriteStruct(ctxAddr, &ctx);

	return res;
}

int sceSdSetMember_(pspChnnlsvContext2& ctx, u8* data, int alignedLen)
{
	if (alignedLen == 0)
	{
		return 0;
	}
	if (alignedLen & 0xF != 0)
	{
		return -1025;
	}
	int i = 0;
	u8 kirkData[20+2048];
	if ((u32)alignedLen >= (u32)2048)
	{
		for(i = 0; alignedLen >= 2048; i += 2048)
		{
			int res = sub_00000000(kirkData, data + i, 2048, ctx.unknown, ctx.unkn, ctx.mode);
			alignedLen = alignedLen - 2048;
			if (res != 0)
			{
				return res;
			}
		}
	}
	if (alignedLen == 0)
	{
		return 0;
	}

	int res = sub_00000000(kirkData, data + i, alignedLen, ctx.unknown, ctx.unkn, ctx.mode);
	return res;
}

int sceChnnlsv_21BE78B4(u32 ctxAddr)
{
	pspChnnlsvContext2 ctx;
	Memory::ReadStruct(ctxAddr, &ctx);

	int res = sceChnnlsv_21BE78B4_(ctx);

	Memory::WriteStruct(ctxAddr, &ctx);
	return res;
}

int sceChnnlsv_21BE78B4_(pspChnnlsvContext2& ctx)
{
	for(int i = 0; i < 16; i++)
	{
		ctx.unknown[i] = 0;
	}
	ctx.unkn = 0;
	ctx.mode = 0;

	return 0;
}

const HLEFunction sceChnnlsv[] =
{
	{0xE7833020,WrapI_UI<sceSdSetIndex>,"sceSdSetIndex"},
	{0xF21A1FCA,WrapI_UUI<sceSdRemoveValue>,"sceSdRemoveValue"},
	{0xC4C494F8,WrapI_UUU<sceSdGetLastIndex>,"sceSdGetLastIndex"},
	{0xABFDFC8B,WrapI_UIIUU<sceSdCreateList>,"sceSdCreateList"},
	{0x850A7FA1,WrapI_UUI<sceSdSetMember>,"sceSdSetMember"},
	{0x21BE78B4,WrapI_U<sceChnnlsv_21BE78B4>,"sceChnnlsv_21BE78B4"},
};

void Register_sceChnnlsv()
{
	RegisterModule("sceChnnlsv",ARRAY_SIZE(sceChnnlsv),sceChnnlsv);
	kirk_init();
}

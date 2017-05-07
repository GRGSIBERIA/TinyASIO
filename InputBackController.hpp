/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once

#include "ControllerBase.hpp"

namespace asio
{
	/*
	 *入力信号を出力にそのまま返す 
	 */
	class InputBackController : public ControllerBase
	{
	private:
		static void BufferSwitch(long index, long)
		{
			void* outBuf = GetOutputMemory(0, index);
			void* inBuf = GetInputMemory(0, index);

			// 入力バッファの内容を出力バッファに書き換えたあと，
			// 入力バッファの内容を入力ストリームに転送する
			TransferMemoryAsStored(Input(0), inBuf, outBuf);
		}

	public:
		/**
		* 指定したチャンネルからコントローラを生成する
		* @param[in] asioDriverName ASIOのドライバ名
		* @param[in] inputChannel 入力を受け付けるチャンネル
		* @param[in] outputChannel 入力された内容を流し込みたい出力チャンネル
		*/
		InputBackController(const std::string& asioDriverName, const InputChannel& inputChannel, const OutputChannel& outputChannel)
			: ControllerBase(asioDriverName) 
		{
			CreateBuffer({inputChannel, outputChannel}, &BufferSwitch);
		}

		/**
		* 0番の入出力チャンネルからコントローラを生成する
		* @note 0番の入出力同士をつなぐので，適当に音の出るチャンネルにジャックを挿してください
		*/
		InputBackController(const std::string& asioDriverName)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({channelManager->Inputs(0), channelManager->Outputs(0)}, &BufferSwitch);
		}

		/**
		* チャンネル番号からコントローラを生成する
		*/
		InputBackController(const std::string& asioDriverName, const long inputNum, const long outputNum)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(inputNum), channelManager->Outputs(outputNum) }, &BufferSwitch);
		}

		/**
		* 入力ストリームに蓄積されたデータを取得する
		* @return 入力ストリームに蓄積されたデータ
		* @note 入力ストリームの内容は空になる
		*/
		StreamPtr Fetch()
		{
			return Input(0).Fetch();
		}

		/**
		* ストリームの現在の長さを得る
		*/
		const long StreamLength() const { return Input(0).StreamLength();}
	};
}
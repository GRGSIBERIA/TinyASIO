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
	class InputOnlyController : public ControllerBase
	{
	private:
		static void BufferSwitch(long index, long)
		{
			void* inBuf = GetInputMemory(0, index);

			Input(0).Store(inBuf, bufferLength);	// 入力ストリームに内容を蓄積する
		}

	public:
		/**
		* 指定したチャンネルからコントローラを生成する
		* @param[in] inputChannel 入力を受け付けるチャンネル
		*/
		InputOnlyController(const std::string& asioDriverName, const InputChannel& inputChannel)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ inputChannel }, &BufferSwitch);
		}

		/**
		* 0番の入出力チャンネルからコントローラを生成する
		* @note 0番の入出力同士をつなぐので，適当に音の出るチャンネルにジャックを挿してください
		*/
		InputOnlyController(const std::string& asioDriverName)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(0) }, &BufferSwitch);
		}

		/**
		* チャンネル番号からコントローラを生成する
		*/
		InputOnlyController(const std::string& asioDriverName, const long inputNum)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(inputNum) }, &BufferSwitch);
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
		const long StreamLength() const { return Input(0).StreamLength(); }
	};
}
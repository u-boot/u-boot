﻿/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Martin Hentschel <info@cl-soft.de>
 *
 */

using System;
using System.Text;
using CCodeGeneration;

namespace LwipSnmpCodeGeneration
{
	public class SnmpScalarNodeCounter64 : SnmpScalarNode
	{
		public SnmpScalarNodeCounter64(SnmpTreeNode parentNode)
			: base(parentNode)
		{
			this.DataType = SnmpDataType.Counter64;
		}

		protected override bool GenerateValueDeclaration(CodeContainerBase container, string variableName, string sourceName)
		{
			container.AddDeclaration(new VariableDeclaration(
				new VariableType(variableName + "_high", LwipDefs.Vt_U32, "*"),
				"(" + new VariableType(null, LwipDefs.Vt_U32, "*").ToString() + ")" + sourceName));
			container.AddDeclaration(new VariableDeclaration(
				new VariableType(variableName + "_low", LwipDefs.Vt_U32, "*"),
				variableName + "_high + 1"));

			container.AddCode(String.Format("LWIP_UNUSED_ARG({0}_high);", variableName));
			container.AddCode(String.Format("LWIP_UNUSED_ARG({0}_low);", variableName));

			return false;
		}

		public override string FixedValueLength
		{
			get { return String.Format("(2 * sizeof({0}))", LwipDefs.Vt_U32); }
		}

		public override int OidRepresentationLen
		{
			get { return 1; }
		}
	}
}

#include "stdafx.h"

#include "RenderSystemDX11Hooks.h"

#include "../shared/AfxDetours.h"

#include <d3d11.h>

#include <map>
#include <list>
#include <shared_mutex>
#include <atomic>

#include <dxgi.h>
#include <dxgi1_4.h>

class CID3D11DeviceContextHook;

std::list<CID3D11DeviceContextHook *> g_D3D11DeviceContextHooks;
std::shared_timed_mutex g_D3D11DeviceContextHooksMutex;

class CID3D11DeviceContextHook : public ID3D11DeviceContext {
public:
    CID3D11DeviceContextHook(ID3D11Device * pDevice,ID3D11DeviceContext * pDeviceContext)
    : m_pDevice(pDevice), m_pDeviceContext(pDeviceContext) {
        pDevice->AddRef();
        {
            std::unique_lock<std::shared_timed_mutex> lock(g_D3D11DeviceContextHooksMutex);
            g_D3D11DeviceContextHooks.emplace_back(this);
        }
    }
    
    ~CID3D11DeviceContextHook() {
        {
            std::unique_lock<std::shared_timed_mutex> lock(g_D3D11DeviceContextHooksMutex);
            g_D3D11DeviceContextHooks.remove(this);
        }
        m_pDevice->Release();
    }    

    void DrawCampath() {
        /*ID3D11RenderTargetView *pRtvs[1];
        m_pDeviceContext->OMGetRenderTargets(1,pRtvs,nullptr);
        if(pRtvs[0]) {
            FLOAT clearColor[4]={1,1,1,1};
            m_pDeviceContext->ClearRenderTargetView(pRtvs[0],clearColor);
            pRtvs[0]->Release();
        }*/
    }

    /*** IUnknown methods ***/

	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {

		return m_pDeviceContext->QueryInterface(riid, ppvObj);
	}


	STDMETHOD_(ULONG,AddRef)(THIS)
	{
		ULONG result = m_pDeviceContext->AddRef();
        
        m_RefCount++;

		return result;
	}

    STDMETHOD_(ULONG,Release)(THIS)
	{
        ULONG result = m_pDeviceContext->Release();
		if(0 == --m_RefCount)
		{
            delete this;
		}
		return result;
	}

    /*** ID3D11DeviceChild methods ***/

	virtual void STDMETHODCALLTYPE GetDevice( 
            /* [annotation] */ 
            _Outptr_  ID3D11Device **ppDevice) { return m_pDeviceContext->GetDevice(ppDevice);  }
    
    virtual HRESULT STDMETHODCALLTYPE GetPrivateData( 
        /* [annotation] */ 
        _In_  REFGUID guid,
        /* [annotation] */ 
        _Inout_  UINT *pDataSize,
        /* [annotation] */ 
        _Out_writes_bytes_opt_( *pDataSize )  void *pData) { return m_pDeviceContext->GetPrivateData(guid,pDataSize,pData);  }
    
    virtual HRESULT STDMETHODCALLTYPE SetPrivateData( 
        /* [annotation] */ 
        _In_  REFGUID guid,
        /* [annotation] */ 
        _In_  UINT DataSize,
        /* [annotation] */ 
        _In_reads_bytes_opt_( DataSize )  const void *pData) { return m_pDeviceContext->SetPrivateData(guid,DataSize,pData);  }
    
    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( 
        /* [annotation] */ 
        _In_  REFGUID guid,
        /* [annotation] */ 
        _In_opt_  const IUnknown *pData) { return m_pDeviceContext->SetPrivateDataInterface(guid,pData);  }
    
    /*** ID3D11DeviceContext methods ***/
        virtual void STDMETHODCALLTYPE VSSetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers) { return m_pDeviceContext->VSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE PSSetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews) { return m_pDeviceContext->PSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  };
        
        virtual void STDMETHODCALLTYPE PSSetShader( 
            /* [annotation] */ 
            _In_opt_  ID3D11PixelShader *pPixelShader,
            /* [annotation] */ 
            _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances) { return m_pDeviceContext->PSSetShader(pPixelShader,ppClassInstances,NumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE PSSetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers) { return m_pDeviceContext->PSSetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE VSSetShader( 
            /* [annotation] */ 
            _In_opt_  ID3D11VertexShader *pVertexShader,
            /* [annotation] */ 
            _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances) { return m_pDeviceContext->VSSetShader(pVertexShader,ppClassInstances,NumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE DrawIndexed( 
            /* [annotation] */ 
            _In_  UINT IndexCount,
            /* [annotation] */ 
            _In_  UINT StartIndexLocation,
            /* [annotation] */ 
            _In_  INT BaseVertexLocation) { return m_pDeviceContext->DrawIndexed(IndexCount,StartIndexLocation,BaseVertexLocation);  }
        
        virtual void STDMETHODCALLTYPE Draw( 
            /* [annotation] */ 
            _In_  UINT VertexCount,
            /* [annotation] */ 
            _In_  UINT StartVertexLocation) { return m_pDeviceContext->Draw(VertexCount,StartVertexLocation);  }
        
        virtual HRESULT STDMETHODCALLTYPE Map( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_  UINT Subresource,
            /* [annotation] */ 
            _In_  D3D11_MAP MapType,
            /* [annotation] */ 
            _In_  UINT MapFlags,
            /* [annotation] */ 
            _Out_opt_  D3D11_MAPPED_SUBRESOURCE *pMappedResource) { return m_pDeviceContext->Map(pResource,Subresource,MapType,MapFlags,pMappedResource);  }
        
        virtual void STDMETHODCALLTYPE Unmap( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_  UINT Subresource) { return m_pDeviceContext->Unmap(pResource,Subresource);  }
        
        virtual void STDMETHODCALLTYPE PSSetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers) { return m_pDeviceContext->PSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE IASetInputLayout( 
            /* [annotation] */ 
            _In_opt_  ID3D11InputLayout *pInputLayout) { return m_pDeviceContext->IASetInputLayout(pInputLayout);  }
        
        virtual void STDMETHODCALLTYPE IASetVertexBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppVertexBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  const UINT *pStrides,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  const UINT *pOffsets) { return m_pDeviceContext->IASetVertexBuffers(StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets);  }
        
        virtual void STDMETHODCALLTYPE IASetIndexBuffer( 
            /* [annotation] */ 
            _In_opt_  ID3D11Buffer *pIndexBuffer,
            /* [annotation] */ 
            _In_  DXGI_FORMAT Format,
            /* [annotation] */ 
            _In_  UINT Offset) { return m_pDeviceContext->IASetIndexBuffer(pIndexBuffer,Format,Offset);  }
        
        virtual void STDMETHODCALLTYPE DrawIndexedInstanced( 
            /* [annotation] */ 
            _In_  UINT IndexCountPerInstance,
            /* [annotation] */ 
            _In_  UINT InstanceCount,
            /* [annotation] */ 
            _In_  UINT StartIndexLocation,
            /* [annotation] */ 
            _In_  INT BaseVertexLocation,
            /* [annotation] */ 
            _In_  UINT StartInstanceLocation) { return m_pDeviceContext->DrawIndexedInstanced(IndexCountPerInstance,InstanceCount,StartIndexLocation,BaseVertexLocation,StartInstanceLocation);  }
        
        virtual void STDMETHODCALLTYPE DrawInstanced( 
            /* [annotation] */ 
            _In_  UINT VertexCountPerInstance,
            /* [annotation] */ 
            _In_  UINT InstanceCount,
            /* [annotation] */ 
            _In_  UINT StartVertexLocation,
            /* [annotation] */ 
            _In_  UINT StartInstanceLocation) { return m_pDeviceContext->DrawInstanced(VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation);  }
        
        virtual void STDMETHODCALLTYPE GSSetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers) { return m_pDeviceContext->GSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE GSSetShader( 
            /* [annotation] */ 
            _In_opt_  ID3D11GeometryShader *pShader,
            /* [annotation] */ 
            _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances) { return m_pDeviceContext->GSSetShader(pShader,ppClassInstances,NumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE IASetPrimitiveTopology( 
            /* [annotation] */ 
            _In_  D3D11_PRIMITIVE_TOPOLOGY Topology) { return m_pDeviceContext->IASetPrimitiveTopology(Topology);  }
        
        virtual void STDMETHODCALLTYPE VSSetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews) { return m_pDeviceContext->VSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE VSSetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers) { return m_pDeviceContext->VSSetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE Begin( 
            /* [annotation] */ 
            _In_  ID3D11Asynchronous *pAsync) { return m_pDeviceContext->Begin(pAsync);  }
        
        virtual void STDMETHODCALLTYPE End( 
            /* [annotation] */ 
            _In_  ID3D11Asynchronous *pAsync) { return m_pDeviceContext->End(pAsync);  }
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
            /* [annotation] */ 
            _In_  ID3D11Asynchronous *pAsync,
            /* [annotation] */ 
            _Out_writes_bytes_opt_( DataSize )  void *pData,
            /* [annotation] */ 
            _In_  UINT DataSize,
            /* [annotation] */ 
            _In_  UINT GetDataFlags) { return m_pDeviceContext->GetData(pAsync,pData,DataSize,GetDataFlags);  }
        
        virtual void STDMETHODCALLTYPE SetPredication( 
            /* [annotation] */ 
            _In_opt_  ID3D11Predicate *pPredicate,
            /* [annotation] */ 
            _In_  BOOL PredicateValue) { return m_pDeviceContext->SetPredication(pPredicate,PredicateValue);  }
        
        virtual void STDMETHODCALLTYPE GSSetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews) { return m_pDeviceContext->GSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE GSSetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers) { return m_pDeviceContext->GSSetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE OMSetRenderTargets( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            _In_opt_  ID3D11DepthStencilView *pDepthStencilView) { return m_pDeviceContext->OMSetRenderTargets(NumViews,ppRenderTargetViews,pDepthStencilView);  }
        
        virtual void STDMETHODCALLTYPE OMSetRenderTargetsAndUnorderedAccessViews( 
            /* [annotation] */ 
            _In_  UINT NumRTVs,
            /* [annotation] */ 
            _In_reads_opt_(NumRTVs)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            _In_opt_  ID3D11DepthStencilView *pDepthStencilView,
            /* [annotation] */ 
            _In_range_( 0, D3D11_1_UAV_SLOT_COUNT - 1 )  UINT UAVStartSlot,
            /* [annotation] */ 
            _In_  UINT NumUAVs,
            /* [annotation] */ 
            _In_reads_opt_(NumUAVs)  ID3D11UnorderedAccessView *const *ppUnorderedAccessViews,
            /* [annotation] */ 
            _In_reads_opt_(NumUAVs)  const UINT *pUAVInitialCounts) { return m_pDeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(NumRTVs,ppRenderTargetViews,pDepthStencilView,UAVStartSlot,NumUAVs,ppUnorderedAccessViews,pUAVInitialCounts);  }
        
        virtual void STDMETHODCALLTYPE OMSetBlendState( 
            /* [annotation] */ 
            _In_opt_  ID3D11BlendState *pBlendState,
            /* [annotation] */ 
            _In_opt_  const FLOAT BlendFactor[ 4 ],
            /* [annotation] */ 
            _In_  UINT SampleMask) { return m_pDeviceContext->OMSetBlendState(pBlendState,BlendFactor,SampleMask);  }
        
        virtual void STDMETHODCALLTYPE OMSetDepthStencilState( 
            /* [annotation] */ 
            _In_opt_  ID3D11DepthStencilState *pDepthStencilState,
            /* [annotation] */ 
            _In_  UINT StencilRef) { return m_pDeviceContext->OMSetDepthStencilState(pDepthStencilState,StencilRef);  }
        
        virtual void STDMETHODCALLTYPE SOSetTargets( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppSOTargets,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  const UINT *pOffsets) { return m_pDeviceContext->SOSetTargets(NumBuffers,ppSOTargets,pOffsets);  }
        
        virtual void STDMETHODCALLTYPE DrawAuto( void) { return m_pDeviceContext->DrawAuto();  }
        
        virtual void STDMETHODCALLTYPE DrawIndexedInstancedIndirect( 
            /* [annotation] */ 
            _In_  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            _In_  UINT AlignedByteOffsetForArgs) { return m_pDeviceContext->DrawIndexedInstancedIndirect(pBufferForArgs,AlignedByteOffsetForArgs);  }
        
        virtual void STDMETHODCALLTYPE DrawInstancedIndirect( 
            /* [annotation] */ 
            _In_  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            _In_  UINT AlignedByteOffsetForArgs) { return m_pDeviceContext->DrawInstancedIndirect(pBufferForArgs,AlignedByteOffsetForArgs);  }
        
        virtual void STDMETHODCALLTYPE Dispatch( 
            /* [annotation] */ 
            _In_  UINT ThreadGroupCountX,
            /* [annotation] */ 
            _In_  UINT ThreadGroupCountY,
            /* [annotation] */ 
            _In_  UINT ThreadGroupCountZ) { return m_pDeviceContext->Dispatch(ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ);  }
        
        virtual void STDMETHODCALLTYPE DispatchIndirect( 
            /* [annotation] */ 
            _In_  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            _In_  UINT AlignedByteOffsetForArgs) { return m_pDeviceContext->DispatchIndirect(pBufferForArgs,AlignedByteOffsetForArgs);  }
        
        virtual void STDMETHODCALLTYPE RSSetState( 
            /* [annotation] */ 
            _In_opt_  ID3D11RasterizerState *pRasterizerState) { return m_pDeviceContext->RSSetState(pRasterizerState);  }
        
        virtual void STDMETHODCALLTYPE RSSetViewports( 
            /* [annotation] */ 
            _In_range_(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
            /* [annotation] */ 
            _In_reads_opt_(NumViewports)  const D3D11_VIEWPORT *pViewports) { return m_pDeviceContext->RSSetViewports(NumViewports,pViewports);  }
        
        virtual void STDMETHODCALLTYPE RSSetScissorRects( 
            /* [annotation] */ 
            _In_range_(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
            /* [annotation] */ 
            _In_reads_opt_(NumRects)  const D3D11_RECT *pRects) { return m_pDeviceContext->RSSetScissorRects(NumRects,pRects);  }
        
        virtual void STDMETHODCALLTYPE CopySubresourceRegion( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            _In_  UINT DstSubresource,
            /* [annotation] */ 
            _In_  UINT DstX,
            /* [annotation] */ 
            _In_  UINT DstY,
            /* [annotation] */ 
            _In_  UINT DstZ,
            /* [annotation] */ 
            _In_  ID3D11Resource *pSrcResource,
            /* [annotation] */ 
            _In_  UINT SrcSubresource,
            /* [annotation] */ 
            _In_opt_  const D3D11_BOX *pSrcBox) { return m_pDeviceContext->CopySubresourceRegion(pDstResource,DstSubresource,DstX,DstY,DstZ,pSrcResource,SrcSubresource,pSrcBox);  }
        
        virtual void STDMETHODCALLTYPE CopyResource( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            _In_  ID3D11Resource *pSrcResource) { return m_pDeviceContext->CopyResource(pDstResource,pSrcResource);  }
        
        virtual void STDMETHODCALLTYPE UpdateSubresource( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            _In_  UINT DstSubresource,
            /* [annotation] */ 
            _In_opt_  const D3D11_BOX *pDstBox,
            /* [annotation] */ 
            _In_  const void *pSrcData,
            /* [annotation] */ 
            _In_  UINT SrcRowPitch,
            /* [annotation] */ 
            _In_  UINT SrcDepthPitch) { return m_pDeviceContext->UpdateSubresource(pDstResource,DstSubresource,pDstBox,pSrcData,SrcRowPitch,SrcDepthPitch);  }
        
        virtual void STDMETHODCALLTYPE CopyStructureCount( 
            /* [annotation] */ 
            _In_  ID3D11Buffer *pDstBuffer,
            /* [annotation] */ 
            _In_  UINT DstAlignedByteOffset,
            /* [annotation] */ 
            _In_  ID3D11UnorderedAccessView *pSrcView) { return m_pDeviceContext->CopyStructureCount(pDstBuffer,DstAlignedByteOffset,pSrcView);  }
        
        virtual void STDMETHODCALLTYPE ClearRenderTargetView( 
            /* [annotation] */ 
            _In_  ID3D11RenderTargetView *pRenderTargetView,
            /* [annotation] */ 
            _In_  const FLOAT ColorRGBA[ 4 ]) { return m_pDeviceContext->ClearRenderTargetView(pRenderTargetView,ColorRGBA);  }
        
        virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewUint( 
            /* [annotation] */ 
            _In_  ID3D11UnorderedAccessView *pUnorderedAccessView,
            /* [annotation] */ 
            _In_  const UINT Values[ 4 ]) { return m_pDeviceContext->ClearUnorderedAccessViewUint(pUnorderedAccessView,Values);  }
        
        virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat( 
            /* [annotation] */ 
            _In_  ID3D11UnorderedAccessView *pUnorderedAccessView,
            /* [annotation] */ 
            _In_  const FLOAT Values[ 4 ]) { return m_pDeviceContext->ClearUnorderedAccessViewFloat(pUnorderedAccessView,Values);  }
        
        virtual void STDMETHODCALLTYPE ClearDepthStencilView( 
            /* [annotation] */ 
            _In_  ID3D11DepthStencilView *pDepthStencilView,
            /* [annotation] */ 
            _In_  UINT ClearFlags,
            /* [annotation] */ 
            _In_  FLOAT Depth,
            /* [annotation] */ 
            _In_  UINT8 Stencil) { return m_pDeviceContext->ClearDepthStencilView(pDepthStencilView,ClearFlags,Depth,Stencil);  }
        
        virtual void STDMETHODCALLTYPE GenerateMips( 
            /* [annotation] */ 
            _In_  ID3D11ShaderResourceView *pShaderResourceView) { return m_pDeviceContext->GenerateMips(pShaderResourceView);  }
        
        virtual void STDMETHODCALLTYPE SetResourceMinLOD( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            FLOAT MinLOD) { return m_pDeviceContext->SetResourceMinLOD(pResource,MinLOD);  }
        
        virtual FLOAT STDMETHODCALLTYPE GetResourceMinLOD( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource) { return m_pDeviceContext->GetResourceMinLOD(pResource);  }
        
        virtual void STDMETHODCALLTYPE ResolveSubresource( 
            /* [annotation] */ 
            _In_  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            _In_  UINT DstSubresource,
            /* [annotation] */ 
            _In_  ID3D11Resource *pSrcResource,
            /* [annotation] */ 
            _In_  UINT SrcSubresource,
            /* [annotation] */ 
            _In_  DXGI_FORMAT Format) { return m_pDeviceContext->ResolveSubresource(pDstResource,DstSubresource,pSrcResource,SrcSubresource,Format);  }
        
        virtual void STDMETHODCALLTYPE ExecuteCommandList( 
            /* [annotation] */ 
            _In_  ID3D11CommandList *pCommandList,
            BOOL RestoreContextState) { return m_pDeviceContext->ExecuteCommandList(pCommandList,RestoreContextState);  }
        
        virtual void STDMETHODCALLTYPE HSSetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews) { return m_pDeviceContext->HSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE HSSetShader( 
            /* [annotation] */ 
            _In_opt_  ID3D11HullShader *pHullShader,
            /* [annotation] */ 
            _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances) { return m_pDeviceContext->HSSetShader(pHullShader,ppClassInstances,NumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE HSSetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers) { return m_pDeviceContext->HSSetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE HSSetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers) { return m_pDeviceContext->HSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE DSSetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews) { return m_pDeviceContext->DSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE DSSetShader( 
            /* [annotation] */ 
            _In_opt_  ID3D11DomainShader *pDomainShader,
            /* [annotation] */ 
            _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances) { return m_pDeviceContext->DSSetShader(pDomainShader,ppClassInstances,NumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE DSSetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers) { return m_pDeviceContext->DSSetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE DSSetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers) { return m_pDeviceContext->DSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE CSSetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews) { return m_pDeviceContext->CSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE CSSetUnorderedAccessViews( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_1_UAV_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_1_UAV_SLOT_COUNT - StartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            _In_reads_opt_(NumUAVs)  ID3D11UnorderedAccessView *const *ppUnorderedAccessViews,
            /* [annotation] */ 
            _In_reads_opt_(NumUAVs)  const UINT *pUAVInitialCounts) { return m_pDeviceContext->CSSetUnorderedAccessViews(StartSlot,NumUAVs,ppUnorderedAccessViews,pUAVInitialCounts);  }
        
        virtual void STDMETHODCALLTYPE CSSetShader( 
            /* [annotation] */ 
            _In_opt_  ID3D11ComputeShader *pComputeShader,
            /* [annotation] */ 
            _In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances) { return m_pDeviceContext->CSSetShader(pComputeShader,ppClassInstances,NumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE CSSetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers) { return m_pDeviceContext->CSSetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE CSSetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers) { return m_pDeviceContext->CSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE VSGetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppConstantBuffers) { return m_pDeviceContext->VSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE PSGetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews) { return m_pDeviceContext->PSGetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE PSGetShader( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11PixelShader **ppPixelShader,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            _Inout_opt_  UINT *pNumClassInstances) { return m_pDeviceContext->PSGetShader(ppPixelShader,ppClassInstances,pNumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE PSGetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _Out_writes_opt_(NumSamplers)  ID3D11SamplerState **ppSamplers) { return m_pDeviceContext->PSGetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE VSGetShader( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11VertexShader **ppVertexShader,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            _Inout_opt_  UINT *pNumClassInstances) { return m_pDeviceContext->VSGetShader(ppVertexShader,ppClassInstances,pNumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE PSGetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppConstantBuffers) { return m_pDeviceContext->PSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE IAGetInputLayout( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11InputLayout **ppInputLayout) { return m_pDeviceContext->IAGetInputLayout(ppInputLayout);  }
        
        virtual void STDMETHODCALLTYPE IAGetVertexBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppVertexBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  UINT *pStrides,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  UINT *pOffsets) { return m_pDeviceContext->IAGetVertexBuffers(StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets);  }
        
        virtual void STDMETHODCALLTYPE IAGetIndexBuffer( 
            /* [annotation] */ 
            _Outptr_opt_result_maybenull_  ID3D11Buffer **pIndexBuffer,
            /* [annotation] */ 
            _Out_opt_  DXGI_FORMAT *Format,
            /* [annotation] */ 
            _Out_opt_  UINT *Offset) { return m_pDeviceContext->IAGetIndexBuffer(pIndexBuffer,Format,Offset);  }
        
        virtual void STDMETHODCALLTYPE GSGetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppConstantBuffers) { return m_pDeviceContext->GSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE GSGetShader( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11GeometryShader **ppGeometryShader,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            _Inout_opt_  UINT *pNumClassInstances) { return m_pDeviceContext->GSGetShader(ppGeometryShader,ppClassInstances,pNumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE IAGetPrimitiveTopology( 
            /* [annotation] */ 
            _Out_  D3D11_PRIMITIVE_TOPOLOGY *pTopology) { return m_pDeviceContext->IAGetPrimitiveTopology(pTopology);  }
        
        virtual void STDMETHODCALLTYPE VSGetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews) { return m_pDeviceContext->VSGetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE VSGetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _Out_writes_opt_(NumSamplers)  ID3D11SamplerState **ppSamplers) { return m_pDeviceContext->VSGetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE GetPredication( 
            /* [annotation] */ 
            _Outptr_opt_result_maybenull_  ID3D11Predicate **ppPredicate,
            /* [annotation] */ 
            _Out_opt_  BOOL *pPredicateValue) { return m_pDeviceContext->GetPredication(ppPredicate,pPredicateValue);  }
        
        virtual void STDMETHODCALLTYPE GSGetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews) { return m_pDeviceContext->GSGetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE GSGetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _Out_writes_opt_(NumSamplers)  ID3D11SamplerState **ppSamplers) { return m_pDeviceContext->GSGetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE OMGetRenderTargets( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11RenderTargetView **ppRenderTargetViews,
            /* [annotation] */ 
            _Outptr_opt_result_maybenull_  ID3D11DepthStencilView **ppDepthStencilView) { return m_pDeviceContext->OMGetRenderTargets(NumViews,ppRenderTargetViews,ppDepthStencilView);  }
        
        virtual void STDMETHODCALLTYPE OMGetRenderTargetsAndUnorderedAccessViews( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumRTVs,
            /* [annotation] */ 
            _Out_writes_opt_(NumRTVs)  ID3D11RenderTargetView **ppRenderTargetViews,
            /* [annotation] */ 
            _Outptr_opt_result_maybenull_  ID3D11DepthStencilView **ppDepthStencilView,
            /* [annotation] */ 
            _In_range_( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT UAVStartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - UAVStartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            _Out_writes_opt_(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews) { return m_pDeviceContext->OMGetRenderTargetsAndUnorderedAccessViews(NumRTVs,ppRenderTargetViews,ppDepthStencilView,UAVStartSlot,NumUAVs,ppUnorderedAccessViews);  }
        
        virtual void STDMETHODCALLTYPE OMGetBlendState( 
            /* [annotation] */ 
            _Outptr_opt_result_maybenull_  ID3D11BlendState **ppBlendState,
            /* [annotation] */ 
            _Out_opt_  FLOAT BlendFactor[ 4 ],
            /* [annotation] */ 
            _Out_opt_  UINT *pSampleMask) { return m_pDeviceContext->OMGetBlendState(ppBlendState,BlendFactor,pSampleMask);  }
        
        virtual void STDMETHODCALLTYPE OMGetDepthStencilState( 
            /* [annotation] */ 
            _Outptr_opt_result_maybenull_  ID3D11DepthStencilState **ppDepthStencilState,
            /* [annotation] */ 
            _Out_opt_  UINT *pStencilRef) { return m_pDeviceContext->OMGetDepthStencilState(ppDepthStencilState,pStencilRef);  }
        
        virtual void STDMETHODCALLTYPE SOGetTargets( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_SO_BUFFER_SLOT_COUNT )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppSOTargets) { return m_pDeviceContext->SOGetTargets(NumBuffers,ppSOTargets);  }
        
        virtual void STDMETHODCALLTYPE RSGetState( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11RasterizerState **ppRasterizerState) { return m_pDeviceContext->RSGetState(ppRasterizerState);  }
        
        virtual void STDMETHODCALLTYPE RSGetViewports( 
            /* [annotation] */ 
            _Inout_ /*_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE )*/   UINT *pNumViewports,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumViewports)  D3D11_VIEWPORT *pViewports) { return m_pDeviceContext->RSGetViewports(pNumViewports,pViewports);  }
        
        virtual void STDMETHODCALLTYPE RSGetScissorRects( 
            /* [annotation] */ 
            _Inout_ /*_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE )*/   UINT *pNumRects,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumRects)  D3D11_RECT *pRects) { return m_pDeviceContext->RSGetScissorRects(pNumRects,pRects);  }
        
        virtual void STDMETHODCALLTYPE HSGetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews) { return m_pDeviceContext->HSGetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE HSGetShader( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11HullShader **ppHullShader,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            _Inout_opt_  UINT *pNumClassInstances) { return m_pDeviceContext->HSGetShader(ppHullShader,ppClassInstances,pNumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE HSGetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _Out_writes_opt_(NumSamplers)  ID3D11SamplerState **ppSamplers) { return m_pDeviceContext->HSGetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE HSGetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppConstantBuffers) { return m_pDeviceContext->HSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE DSGetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews) { return m_pDeviceContext->DSGetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE DSGetShader( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11DomainShader **ppDomainShader,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            _Inout_opt_  UINT *pNumClassInstances) { return m_pDeviceContext->DSGetShader(ppDomainShader,ppClassInstances,pNumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE DSGetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _Out_writes_opt_(NumSamplers)  ID3D11SamplerState **ppSamplers) { return m_pDeviceContext->DSGetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE DSGetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppConstantBuffers) { return m_pDeviceContext->DSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE CSGetShaderResources( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            _Out_writes_opt_(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews) { return m_pDeviceContext->CSGetShaderResources(StartSlot,NumViews,ppShaderResourceViews);  }
        
        virtual void STDMETHODCALLTYPE CSGetUnorderedAccessViews( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_1_UAV_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_1_UAV_SLOT_COUNT - StartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            _Out_writes_opt_(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews) { return m_pDeviceContext->CSGetUnorderedAccessViews(StartSlot,NumUAVs,ppUnorderedAccessViews);  }
        
        virtual void STDMETHODCALLTYPE CSGetShader( 
            /* [annotation] */ 
            _Outptr_result_maybenull_  ID3D11ComputeShader **ppComputeShader,
            /* [annotation] */ 
            _Out_writes_opt_(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            _Inout_opt_  UINT *pNumClassInstances) { return m_pDeviceContext->CSGetShader(ppComputeShader,ppClassInstances,pNumClassInstances);  }
        
        virtual void STDMETHODCALLTYPE CSGetSamplers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            _Out_writes_opt_(NumSamplers)  ID3D11SamplerState **ppSamplers) { return m_pDeviceContext->CSGetSamplers(StartSlot,NumSamplers,ppSamplers);  }
        
        virtual void STDMETHODCALLTYPE CSGetConstantBuffers( 
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            _In_range_( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            _Out_writes_opt_(NumBuffers)  ID3D11Buffer **ppConstantBuffers) { return m_pDeviceContext->CSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);  }
        
        virtual void STDMETHODCALLTYPE ClearState( void) { return m_pDeviceContext->ClearState();  }
        
        virtual void STDMETHODCALLTYPE Flush( void) { return m_pDeviceContext->Flush();  }
        
        virtual D3D11_DEVICE_CONTEXT_TYPE STDMETHODCALLTYPE GetType( void) { return m_pDeviceContext->GetType();  }
        
        virtual UINT STDMETHODCALLTYPE GetContextFlags( void) { return m_pDeviceContext->GetContextFlags();  }
        
        virtual HRESULT STDMETHODCALLTYPE FinishCommandList( 
            BOOL RestoreDeferredContextState,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11CommandList **ppCommandList) { return m_pDeviceContext->FinishCommandList(RestoreDeferredContextState,ppCommandList);  }
        


protected:

private:
    ID3D11Device * m_pDevice;
    ID3D11DeviceContext * m_pDeviceContext;
    std::atomic_size_t m_RefCount = 1;
};

IDXGISwapChain * g_pSwapChain = nullptr;
ID3D11Device * g_pDevice = nullptr;
ID3D11DeviceContext * g_pImmediateContext = nullptr;
ID3D11RenderTargetView * g_pRTView = nullptr;

typedef HRESULT (STDMETHODCALLTYPE * CreateRenderTargetView_t)( void * This,
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView);

CreateRenderTargetView_t g_Old_CreateRenderTargetView = nullptr;


HRESULT STDMETHODCALLTYPE New_CreateRenderTargetView(  void * This,
            /* [annotation] */ 
            _In_  ID3D11Resource *pResource,
            /* [annotation] */ 
            _In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            _COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView) {
    
    HRESULT result = g_Old_CreateRenderTargetView(This, pResource, pDesc, ppRTView);

    if(SUCCEEDED(result) && ppRTView && *ppRTView) {
        ID3D11Texture2D * pTexture = nullptr;
        HRESULT result2 = g_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D), (void**)&pTexture);
        if(SUCCEEDED(result2)) {
            if(pResource == pTexture) {
                g_pRTView = *ppRTView;
            }

            pTexture->Release();
        }
    }

    return result;
}

typedef void (STDMETHODCALLTYPE * ID3D11Device_GetImmediateContext_t)(ID3D11Device * This, 
    /* [annotation] */ 
    _Outptr_  ID3D11DeviceContext **ppImmediateContext);

ID3D11Device_GetImmediateContext_t g_Old_ID3D11Device_GetImmediateContext = nullptr;

void STDMETHODCALLTYPE New_ID3D11Device_GetImmediateContext_t(ID3D11Device * This, 
    /* [annotation] */ 
    _Outptr_  ID3D11DeviceContext **ppImmediateContext) {
    g_Old_ID3D11Device_GetImmediateContext(This, ppImmediateContext);
    if(ppImmediateContext && *ppImmediateContext){
        //*ppImmediateContext = new CID3D11DeviceContextHook(This,*ppImmediateContext);
        g_pImmediateContext = *ppImmediateContext;
    }
}

HRESULT WINAPI new_D3D11CreateDevice(IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device **ppDevice, D3D_FEATURE_LEVEL *pFeatureLevel, ID3D11DeviceContext **ppImmediateContext);

CAfxImportFuncHook<HRESULT(WINAPI*)(IDXGIAdapter*,D3D_DRIVER_TYPE,HMODULE,UINT,const D3D_FEATURE_LEVEL*,UINT,UINT,ID3D11Device**,D3D_FEATURE_LEVEL*,ID3D11DeviceContext**)> g_Import_rendersystemdx11_d3d11_D3D11CreateDevice("D3D11CreateDevice", &new_D3D11CreateDevice);
CAfxImportDllHook g_Import_rendersystemdx11_d3d11("d3d11.dll", CAfxImportDllHooks({
	&g_Import_rendersystemdx11_d3d11_D3D11CreateDevice
    }));

HRESULT WINAPI new_D3D11CreateDevice(
    IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext
    ) {
    HRESULT result = g_Import_rendersystemdx11_d3d11_D3D11CreateDevice.GetTrueFuncValue()(
        pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

    if(SUCCEEDED(result) && ppDevice && *ppDevice) {
        g_pDevice = *ppDevice;
        if(nullptr == g_Old_ID3D11Device_GetImmediateContext) {
            void **vtable = *(void***)*ppDevice;
            AfxDetourPtr(&(vtable[9]),New_CreateRenderTargetView,(PVOID*)&g_Old_CreateRenderTargetView);
        }
        if(nullptr == g_Old_CreateRenderTargetView) {
            void **vtable = *(void***)*ppDevice;
            AfxDetourPtr(&(vtable[40]),New_ID3D11Device_GetImmediateContext_t,(PVOID*)&g_Old_ID3D11Device_GetImmediateContext);
        }
    }
    if(SUCCEEDED(result) && ppImmediateContext && *ppImmediateContext) {
        g_pImmediateContext = *ppImmediateContext;
        //*ppImmediateContext = new CID3D11DeviceContextHook(*ppDevice,*ppImmediateContext); // They will just release it shortly there-after, which is why we detour ID3D11Device::GetImmediateContext.
    }

    return result;
}

typedef HRESULT (STDMETHODCALLTYPE * Present_t)( void * This,
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags);

Present_t g_OldPresent = nullptr;

HRESULT STDMETHODCALLTYPE New_Present( void * This,
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags) {

    ID3D11RenderTargetView *pRtvs[1] = {nullptr};
    ID3D11DepthStencilView *pDsvs[1] = {nullptr};
    g_pImmediateContext->OMGetRenderTargets(1,pRtvs,pDsvs);
    if(g_pRTView) {
        ID3D11RenderTargetView *pRtvs2[1] = {g_pRTView};
        g_pImmediateContext->OMSetRenderTargets(1,pRtvs2,nullptr);
        FLOAT clearColor[4]={0.5f, 0.0f, 0.0f, 0.5f};
        g_pImmediateContext->ClearRenderTargetView(g_pRTView,clearColor);
    }
    g_pImmediateContext->OMSetRenderTargets(1,pRtvs,(pDsvs[0]?pDsvs[0]:nullptr));
    if(pRtvs[0]) pRtvs[0]->Release();
    if(pDsvs[0]) pDsvs[0]->Release();

    HRESULT result = g_OldPresent(This, SyncInterval, Flags);
    return result;
}


typedef HRESULT (STDMETHODCALLTYPE * CreateSwapChain_t)( void * This,
            /* [annotation][in] */ 
            _In_  IUnknown *pDevice,
            /* [annotation][in] */ 
            _In_  DXGI_SWAP_CHAIN_DESC *pDesc,
            /* [annotation][out] */ 
            _COM_Outptr_  IDXGISwapChain **ppSwapChain);

CreateSwapChain_t g_OldCreateSwapChain = nullptr;

HRESULT STDMETHODCALLTYPE New_CreateSwapChain( void * This,
            /* [annotation][in] */ 
            _In_  IUnknown *pDevice,
            /* [annotation][in] */ 
            _In_  DXGI_SWAP_CHAIN_DESC *pDesc,
            /* [annotation][out] */ 
            _COM_Outptr_  IDXGISwapChain **ppSwapChain) {
    HRESULT result = g_OldCreateSwapChain(This, pDevice, pDesc, ppSwapChain);

    if(SUCCEEDED(result) && ppSwapChain && *ppSwapChain) {
        g_pSwapChain = *ppSwapChain;
        if(nullptr == g_OldPresent) {
            void **vtable = *(void***)*ppSwapChain;
            AfxDetourPtr(&(vtable[8]),New_Present,(PVOID*)&g_OldPresent);
        }
    }

    return result;
}

HRESULT WINAPI New_CreateDXGIFactory(REFIID riid, _COM_Outptr_ void **ppFactory);

CAfxImportFuncHook<HRESULT(WINAPI*)(REFIID riid, _COM_Outptr_ void **ppFactory)> g_Import_rendersystemdx11_dxgi_CreateDXGIFactory("CreateDXGIFactory", &New_CreateDXGIFactory);
CAfxImportDllHook g_Import_rendersystemdx11_dxgi("dxgi.dll", CAfxImportDllHooks({
	&g_Import_rendersystemdx11_dxgi_CreateDXGIFactory
    }));

HRESULT WINAPI New_CreateDXGIFactory(REFIID riid, _COM_Outptr_ void **ppFactory) {
    HRESULT result = g_Import_rendersystemdx11_dxgi_CreateDXGIFactory.GetTrueFuncValue()(riid, ppFactory);

    if(SUCCEEDED(result) && ppFactory && *ppFactory
        && ( __uuidof(IDXGIFactory4) == riid || __uuidof(IDXGIFactory) == riid )
        && nullptr == g_OldCreateSwapChain) {
            void **vtable = *(void***)*ppFactory;
            AfxDetourPtr(&(vtable[10]),New_CreateSwapChain,(PVOID*)&g_OldCreateSwapChain);
        }
        
    return result;
}

CAfxImportsHook g_Import_rendersystemdx11(CAfxImportsHooks({
	&g_Import_rendersystemdx11_d3d11,
	&g_Import_rendersystemdx11_dxgi
    }));

bool Hook_RenderSystemDX11(void * hModule) {
    static bool firstResult = false;
    static bool firstRun = true;

    if(firstRun) {
        firstRun = false;

        if(g_Import_rendersystemdx11.Apply((HMODULE)hModule)) {
            firstResult = true;
        }
    }

    return firstResult;
}

void DrawCampath() {
    std::shared_lock<std::shared_timed_mutex> lock(g_D3D11DeviceContextHooksMutex);
    auto it = g_D3D11DeviceContextHooks.begin();
    if(it != g_D3D11DeviceContextHooks.end()) {
        (*it)->DrawCampath();
    }
}

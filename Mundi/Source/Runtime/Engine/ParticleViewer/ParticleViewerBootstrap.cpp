#include "pch.h"
#include "ParticleViewerBootstrap.h"
#include "CameraActor.h"
#include "Source/Runtime/Engine/SkeletalViewer/ViewerState.h"
#include "FViewport.h"
#include "FSkeletalViewerViewportClient.h"
#include "Source/Runtime/AssetManagement/ResourceManager.h"

ViewerState* ParticleViewerBootstrap::CreateViewerState(const char* Name, UWorld* InWorld, ID3D11Device* InDevice)
{
    if (!InDevice) return nullptr;

    ViewerState* State = new ViewerState();
    State->Name = Name ? Name : "Particle Viewer";

    // Preview world 만들기
    State->World = NewObject<UWorld>();
    State->World->SetWorldType(EWorldType::PreviewMinimal);  // Set as preview world for memory optimization
    State->World->Initialize();
    State->World->GetRenderSettings().DisableShowFlag(EEngineShowFlags::SF_EditorIcon);
   // State->World->GetRenderSettings().DisableShowFlag(EEngineShowFlags::SF_Gizmo);  // Hide gizmo in particle editor

    State->World->GetGizmoActor()->SetSpace(EGizmoSpace::Local);

    // Viewport + client per tab
    State->Viewport = new FViewport();
    // 프레임 마다 initial size가 바뀔 것이다
    State->Viewport->Initialize(0, 0, 1, 1, InDevice);

    auto* Client = new FSkeletalViewerViewportClient();
    Client->SetWorld(State->World);
    Client->SetViewportType(EViewportType::Perspective);
    Client->SetViewMode(EViewMode::VMI_Lit_Phong);
    Client->GetCamera()->SetActorLocation(FVector(3, 0, 2));

    State->Client = Client;
    State->Viewport->SetViewportClient(Client);

    State->World->SetEditorCameraActor(Client->GetCamera());

    // 파티클 시스템 프리뷰를 위한 액터는 필요시 나중에 추가
    State->PreviewActor = nullptr;

    return State;
}

void ParticleViewerBootstrap::DestroyViewerState(ViewerState*& State)
{
    if (!State) return;

    if (State->Viewport) { delete State->Viewport; State->Viewport = nullptr; }
    if (State->Client) { delete State->Client; State->Client = nullptr; }
    if (State->World) { ObjectFactory::DeleteObject(State->World); State->World = nullptr; }
    delete State; State = nullptr;
}

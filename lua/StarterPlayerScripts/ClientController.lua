local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Workspace = game:GetService("Workspace")

local cameraEvent = ReplicatedStorage:WaitForChild("CameraCommand")

cameraEvent.OnClientEvent:Connect(function(action, data)
	local camera = Workspace.CurrentCamera

	print("[Camera Controller] Executing:", action, "with data:", data)

	local success, err = pcall(function()
		if action == "set_camera_fov" then
			camera.FieldOfView = data.value

		elseif action == "set_camera_position" then
			camera.CameraType = Enum.CameraType.Scriptable
			camera.CFrame = CFrame.new(data.x, data.y, data.z) * camera.CFrame.Rotation

		elseif action == "set_camera_rotation" then
			camera.CameraType = Enum.CameraType.Scriptable
			local pos = camera.CFrame.Position
			camera.CFrame = CFrame.new(pos) * CFrame.Angles(math.rad(data.x), math.rad(data.y), math.rad(data.z))
		end
	end)

	if success then
		print("[Camera Controller] Command completed:", action)
	else
		warn("[Camera Controller] Command failed:", action, err)
	end
end)